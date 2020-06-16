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
	CELLThread m_thread;
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
#else
		//�����쳣�źţ�Ĭ������ᵼ�½�����ֹ
		if (signal(SIGPIPE,SIG_IGN) == SIG_ERR)
		{
			return 1;
		}
#endif //_WIN32

        if (INVALID_SOCKET != m_sock)
        {
            CELLLog::Info("�ر�֮ǰ���ӣ�socket = %d\n", m_sock);
            Close();
        }
        m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (INVALID_SOCKET == m_sock)
        {
            CELLLog::Info("socket = %d����ʧ��\n", m_sock);
        }
        else
        {
            CELLLog::Info("socket = %d�����ɹ�\n", m_sock);
        }
        return m_sock;
    }
    //�󶨶˿ں�
    int Bind(const char* ip, unsigned short port)
    {
        if (INVALID_SOCKET == m_sock)
        {
            CELLLog::Info("��ʼ��socket\n");
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
            CELLLog::Info("ERROR,�󶨶˿�ʧ��,%d��\n", port);
        }
        else
        {
            CELLLog::Info("SUCCESS,�󶨶˿ڳɹ�:%d��\n", port);
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
            CELLLog::Info("ERROR,socket = %d����ʧ�ܣ�\n", m_sock);
        }
        else
        {
            CELLLog::Info("SUCCESS,socket = %d�����ɹ���\n", m_sock);
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
            CELLLog::Info("socket = %d���󣬽��ܵĿͻ���SOCKET ��Ч\n", m_sock);
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
            //CELLLog::Info("�¿ͻ��˼���,sock = %d,ip = %s���ͻ����� = %d \n", _cSock, inet_ntoa(clientA.sin_addr), m_vectClients.size());
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
		//�߳�
		m_thread.Start(nullptr, [this](CELLThread *pThread) {
			OnRun(pThread);
		});
    }

  
    //�ر�
    void Close()
    {
		CELLLog::Info("EasyTcpServerClose begin \n");
		m_thread.Close();
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
		
		CELLLog::Info("EasyTcpServerClose end \n");
    }
    //���㲢���ÿ�����Ϣ����
    void time4msg()
    {
        auto t1 = m_tTime.getElaspedSecond();
        if (t1 >= 1.0)
        {
            CELLLog::Info("thread<%d>,time<%lf>,socket<%d>,clientNum<%d>,recvCount<%d>��msgCount<%d>\n", m_vectServers.size(), t1, m_sock,(int)m_clientCount,(int)(m_recvCount / t1),(int)(m_msgCount / t1));
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
	//����������Ϣ
	void OnRun(CELLThread *pThread)
	{
		while (pThread->isRun())
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
			timeval tl = { 0,1};
			int ret = select(m_sock + 1, &fd_read, 0, 0, &tl);
			//CELLLog::Info("select ret = %d,count  = %d\n",ret, _count++);
			if (ret < 0)
			{
				CELLLog::Info("EasyTcpServer::OnRun,select����������˳�\n");
				pThread->Exit();
				break;
			}
			if (FD_ISSET(m_sock, &fd_read))
			{
				FD_CLR(m_sock, &fd_read);
				Accept();
			}
		}
	}
};


#endif // !_EASYTCPSERVER_HPP

