#ifndef _EASYTCPSERVER_HPP
#define _EASYTCPSERVER_HPP

#include "Cell.hpp"
#include "CellClient.hpp"

#include "INetEvent.hpp"
#include "CellServer.hpp"

#include <stdio.h>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>


//�ͻ��˽��շ���
class EasyTcpServer :public INetEvent
{
private:
    SOCKET m_sock;
    //�����б�
    std::vector<std::shared_ptr<CellServer> > m_vectServers;
    //ÿ����Ϣ��ʱ
    CELLTimestamp m_tTime;
protected:
    //�ͻ��˼���
    std::atomic_int m_clientCount;
	//���յ���Ϣ����
	std::atomic_int m_recvCount;
	//��Ϣ����
	std::atomic_int  m_msgCount;
public:
    EasyTcpServer()
    {
        m_sock = INVALID_SOCKET;
        m_clientCount = 0;
		m_recvCount = 0;
		m_msgCount = 0;
    }
    virtual ~EasyTcpServer()
    {
        Close();
    }
    //��ʼ��
    SOCKET initSocket()
    {
        //���� Win sock 2.x
#ifdef _WIN32
        WORD ver = MAKEWORD(2, 2);
        WSADATA data;
        WSAStartup(ver, &data);
#endif
        if (INVALID_SOCKET != m_sock)
        {
            printf("�ر�֮ǰ���ӣ�socket = %d\n", m_sock);
            Close();
        }
        m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (INVALID_SOCKET == m_sock)
        {
            printf("socket = %d����ʧ��\n", m_sock);
        }
        else
        {
            printf("socket = %d�����ɹ�\n", m_sock);
        }
        return m_sock;
    }
    //�󶨶˿ں�
    int Bind(const char* ip, unsigned short port)
    {
        if (INVALID_SOCKET == m_sock)
        {
            printf("��ʼ��socket\n");
            initSocket();
        }
        //bind
        sockaddr_in _sin = {};
        _sin.sin_family = AF_INET;
        _sin.sin_port = htons(port);
#ifdef _WIN32
        if (ip)
        {
            _sin.sin_addr.S_un.S_addr = inet_addr(ip);
        }
        else
        {
            _sin.sin_addr.S_un.S_addr = INADDR_ANY;
        }
#else
        if (ip)
        {
            sin.sin_addr.s_addr = inet_addr(ip);
        }
        else
        {
            sin.sin_addr.s_addr = INADDR_ANY;
        }
#endif
        int ret = bind(m_sock, (sockaddr*)&_sin, sizeof(_sin));
        if (SOCKET_ERROR == ret)
        {
            printf("ERROR,�󶨶˿�ʧ��,%d��\n", port);
        }
        else
        {
            printf("SUCCESS,�󶨶˿ڳɹ�:%d��\n", port);
        }
        return ret;
    }
    //����
    int Listen(int n)
    {
        //listen
        int ret = listen(m_sock, n);
        if (SOCKET_ERROR == ret)
        {
            printf("ERROR,socket = %d����ʧ�ܣ�\n", m_sock);
        }
        else
        {
            printf("SUCCESS,socket = %d�����ɹ���\n", m_sock);
        }
        return ret;
    }
    //accept
    SOCKET Accept()
    {
        //accept
        sockaddr_in clientA = {};
#ifdef _WIN32
        int nClientA = sizeof(clientA);
#else
        socklen_t nClientA = sizeof(clientA);
#endif // _WIN32


        SOCKET _cSock = INVALID_SOCKET;
        char msgBuf[] = "Hello.I am Server!";

        _cSock = accept(m_sock, (sockaddr*)&clientA, &nClientA);
        if (INVALID_SOCKET == _cSock)
        {
            printf("socket = %d���󣬽��ܵĿͻ���SOCKET ��Ч\n", m_sock);
        }
        else
        {
            //NewUserJoin userJoin;
            //userJoin.sock = _cSock;
            //SendDataToAll((DataHeader*)&userJoin);
			//ֱ��make_shared�޷��������� 
			std::shared_ptr<CellClient> c(new CellClient(_cSock));
			addClientToCellServer(c);
           // addClientToCellServer(std::make_shared<CellClient>(_cSock));
            //m_vectClients.push_back(new CellClient(_cSock));
            //send
            //printf("�¿ͻ��˼���,sock = %d,ip = %s���ͻ����� = %d \n", _cSock, inet_ntoa(clientA.sin_addr), m_vectClients.size());
        }
        return _cSock;
    }

    //��ѯ��С�ͻ������߳�
    void addClientToCellServer(std::shared_ptr<CellClient> pClient)
    {
        //��ѯ�ͻ������ٵ���Ϣ�����߳�
        auto pMinCellServer = m_vectServers[0];
        for (auto pCellServer : m_vectServers)
        {
            if (pCellServer->getClientNum() < pMinCellServer->getClientNum())
            {
				pMinCellServer = pCellServer;
            }
        }
		pMinCellServer->addClient(pClient);
    }
    void Start(int nCellCount)
    {
        for (int n = 0; n < nCellCount; n++)
        {
            auto ser = std::make_shared<CellServer>(n + 1);
            m_vectServers.push_back(ser);
            //ע�������¼����ܶ���
            ser->setEventObj(this);
            //������Ϣ��������߳�
            ser->Start();
        }
    }
    bool isRun()
    {
        return m_sock != INVALID_SOCKET;
    }
    //����������Ϣ
    bool OnRun()
    {
        if (isRun())
        {
            time4msg();
            fd_set fd_read;
            fd_set fd_write;
            fd_set fd_except;

            FD_ZERO(&fd_read);
            //FD_ZERO(&fd_write);
            //FD_ZERO(&fd_except);

            FD_SET(m_sock, &fd_read);
            //FD_SET(m_sock, &fd_write);
            //FD_SET(m_sock, &fd_except);


            //nfds��һ������ֵ����ָfd_set����������������(socket)�ķ�Χ
            //�����������������ֵ+1����windows�������������д0
            timeval tl = { 0,10 };
            int ret = select(m_sock + 1, &fd_read, 0, 0, &tl);
            //printf("select ret = %d,count  = %d\n",ret, _count++);
            if (ret < 0)
            {
                printf("select����������˳�\n");
                return false;
            }
            if (FD_ISSET(m_sock, &fd_read))
            {
                FD_CLR(m_sock, &fd_read);
                Accept();
                return true;
            }
            return true;
        }
        return false;

    }
    //�ر�
    void Close()
    {
		printf("EasyTcpServerClose begin \ n");
        //�������
        if (m_sock != INVALID_SOCKET)
        {
			m_vectServers.clear();
#ifdef _WIN32
            closesocket(m_sock);
            WSACleanup();
#else
            close(m_sock);
#endif
            m_sock = INVALID_SOCKET;
        }
		printf("EasyTcpServerClose end \ n");
    }
    //���㲢���ÿ�����Ϣ����
    void time4msg()
    {
        auto t1 = m_tTime.getElaspedSecond();
        if (t1 >= 1.0)
        {
            printf("thread<%d>,time<%lf>,socket<%d>,clientNum<%d>,recvCount<%d>��msgCount<%d>\n", m_vectServers.size(), t1, m_sock, m_clientCount, (int)(m_recvCount / t1),(int)(m_msgCount / t1));
			m_recvCount = 0;
			m_msgCount = 0;
            m_tTime.update();
        }
    }

    

    virtual void OnNetLeave(std::shared_ptr<CellClient> pClient)
    {
        m_clientCount--;
    }
	virtual void OnNetMsg(CellServer* pCellServer, std::shared_ptr<CellClient> pClient, DataHeader*  header)
    {
		m_msgCount++;
    }
	virtual void OnNetJoin(std::shared_ptr<CellClient> pClient)
	{
		m_clientCount++;
	}
	virtual void OnNetRecv(std::shared_ptr<CellClient> pClient)
	{
		m_recvCount++;
	}
private:

};


#endif // !_EASYTCPSERVER_HPP

