#ifndef _EASYTCPSERVER_HPP
#define _EASYTCPSERVER_HPP
#ifdef _WIN32
#define FD_SETSIZE 10024
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#define SOCKET  int
#define INVALID_SOCKET (SOCKET)(0)
#define SOCKET_ERROR (-1)
#endif
#include <stdio.h>
#include "MessageHeader.hpp"
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include  "CELLTimestamp.hpp"
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240//��������С
#endif // !RECV_BUFF_SIZE

//�ͻ������ݶ���
class ClientSocket
{
public:
    ClientSocket(SOCKET sock = INVALID_SOCKET)
    {
        m_sockfd = sock;
        memset(m_szMSGBuf, 0, sizeof(m_szMSGBuf));
        m_lastPos = 0;
    }

    SOCKET getSocket()
    {
        return m_sockfd;
    }
    char *msgBuf()
    {
        return m_szMSGBuf;
    }

    int getLast()
    {
        return m_lastPos;
    }

    void setLast(int last)
    {
        m_lastPos = last;
    }

	//����SOCKET����
	int SendData(DataHeader *header)
	{
		if ( header)
		{
			return send(m_sockfd, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
private:
    SOCKET m_sockfd;
    //������
    char m_szMSGBuf[RECV_BUFF_SIZE * 2];
    //��Ϣ������β��λ��
    int  m_lastPos = 0;
};
//�����¼��ӿ�
class INetEvent
{
public:
    //���麯��
	//�ͻ��˼���ʱ֪ͨ���ͻ����뿪�¼�
	virtual void OnNetJoin(ClientSocket* pClient) = 0;
    //�ͻ����뿪ʱ֪ͨ���ͻ����뿪�¼�
    virtual void OnNetLeave(ClientSocket* pClient) = 0;
    //�ͻ��˶��յ���Ϣ��֪ͨ���߳�
    virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header) = 0;
private:

};
class CellSercer
{
public:
    CellSercer(SOCKET sock = INVALID_SOCKET)
    {
        m_sock = sock;
        m_pthread = NULL;
        

        m_pInetEvent = NULL;
    }
    ~CellSercer()
    {
        Close();
        m_sock = INVALID_SOCKET;
        delete m_pthread;
        m_pthread = NULL;
        delete m_pInetEvent;
        m_pInetEvent = NULL;
    }

    void setEventObj(INetEvent* pObj)
    {
        m_pInetEvent = pObj;
    }
    //����������Ϣ
    bool OnRun()
    {
        while (isRun())
        {
            if (m_vectClientsBuff.size() > 0)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                for (auto pClient : m_vectClientsBuff)
                {
                    m_vectClients.push_back(pClient);
                }
                m_vectClientsBuff.clear();
            }
            //���û����Ҫ����ÿͻ��˾�����
            if (m_vectClients.empty())
            {
                std::chrono::milliseconds t(1);
                std::this_thread::sleep_for(t);
                continue;
            }
            fd_set fd_read;
            FD_ZERO(&fd_read);
            SOCKET maxSoc = m_vectClients[0]->getSocket();
            for (int i = (int)m_vectClients.size() - 1; i >= 0; i--)
            {
                FD_SET(m_vectClients[i]->getSocket(), &fd_read);
                if (maxSoc  < m_vectClients[i]->getSocket())
                {
                    maxSoc = m_vectClients[i]->getSocket();
                }
            }
            //nfds��һ������ֵ����ָfd_set����������������(socket)�ķ�Χ
            //�����������������ֵ+1����windows�������������д0
            int ret = select(maxSoc + 1, &fd_read, nullptr, nullptr, nullptr);
            //printf("select ret = %d,count  = %d\n",ret, _count++);
            if (ret < 0)
            {
                printf("select����������˳�\n");
                return false;
            }

            auto it = m_vectClients.begin();
            while (it != m_vectClients.end())
            {
                if (FD_ISSET((*it)->getSocket(), &fd_read))
                {
                    if (-1 == RecvData(*it))
                    {
                        if (m_pInetEvent)
                        {
                            m_pInetEvent->OnNetLeave(*it);
                        }
                        delete (*it);
                        it = m_vectClients.erase(it);
                    }
                    else
                    {
                        it++;
                    }
                }
                else
                {
                    it++;
                }
            }
            //return true;
        }
        //return false;
    }

    //
    bool isRun()
    {
        return m_sock != INVALID_SOCKET;
    }
    int _count = 0;


    int RecvData(ClientSocket *pClient)
    {

        //���տͻ��˵���������
        int nLen = recv(pClient->getSocket(), recvBUF, RECV_BUFF_SIZE, 0);
        //printf("Recv len = %d\n", nLen);
        //DataHeader *header = (DataHeader*)recvBUF;
        if (nLen < 0)
        {
            //printf("�ͻ���<socket = %d>���˳��������������\n", pClient->getSocket());
            return -1;
        }

        memcpy(pClient->msgBuf() + pClient->getLast(), recvBUF, nLen);
        //��Ϣ������β����λ�ú���
        pClient->setLast(pClient->getLast() + nLen);
        //�ж�������Ϣ�����������ݳ����Ƿ������Ϣͷ
        while (pClient->getLast() >= sizeof(DataHeader))
        {
            DataHeader* header = (DataHeader*)pClient->msgBuf();
            if (pClient->getLast() >= header->dataLength)
            {
                //ʣ��δ�������Ϣ���������ݵĳ���
                int nSize = pClient->getLast() - header->dataLength;
                //����������Ϣ
                OnNetMsg(pClient, header);
                //��δ���������ǰ��
                memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
                pClient->setLast(nSize);;
            }
            else
            {
                break;
            }
        }

        //LoginResult ret;
        //SendData(pClient->getSocket(), &ret);
        /* recv(_cSock, recvBUF + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
        OnNetMsg(_cSock, header);*/
        return 0;
    }
    void OnNetMsg(ClientSocket* pClient, DataHeader* header)
    {
        //����ͻ�������
		m_pInetEvent->OnNetMsg(pClient, header);
        /*auto t1 = m_tTime.getElaspedSecond();
        if (t1 >= 1.0)
        {
        printf("time<%lf>,socket<%d>,clientNum<%d>,recvCount<%d>\n", t1, _cSock, m_vectClients.size(), m_recvCount);
        m_tTime.update();
        m_recvCount = 0;
        }*/
    }


    //�ر�
    void Close()
    {
        //�������
        if (m_sock != INVALID_SOCKET)
        {
#ifdef _WIN32
            for (int i = m_vectClients.size() - 1; i >= 0; i--)
            {
                closesocket(m_vectClients[i]->getSocket());
                delete (m_vectClients[i]);
            }
#else
            for (size_t i = m_vectClients.size() - 1; i >= 0; i--)
            {
                close(m_vectClients[i]->getSocket());
                delete m_vectClients[i];
            }
#endif
            m_sock = INVALID_SOCKET;
            m_vectClients.clear();
        }
    }

    void addClient(ClientSocket* pClient)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        //m_mutex.lock();
        m_vectClientsBuff.push_back(pClient);
        //	m_mutex.unlock;
    }

    void Start()
    {
        m_pthread = new std::thread(std::mem_fn(&CellSercer::OnRun), this);
    }

    size_t getClientNum()
    {
        return m_vectClients.size() + m_vectClientsBuff.size();
    }
private:
    SOCKET m_sock;
    //�ͻ�����ʽ����
    std::vector<ClientSocket*> m_vectClients;
    std::vector<ClientSocket*> m_vectClientsBuff;//�ͻ��˻������
    std::mutex m_mutex;//���ӿͻ�����,���������
    std::thread *m_pthread;
    //�����¼�����
    INetEvent* m_pInetEvent;
    //��������
    char recvBUF[RECV_BUFF_SIZE] = {};
public:
};
class EasyTcpServer :public INetEvent
{
private:
    SOCKET m_sock;
    //�����б�
    std::vector<CellSercer*> m_vectServers;
    //ÿ����Ϣ��ʱ
    CELLTimestamp m_tTime;
protected:
    //�ͻ��˼���
    std::atomic_int m_clientCount;
	//��Ϣ����
	std::atomic_int m_recvCount;
public:
    EasyTcpServer()
    {
        m_sock = INVALID_SOCKET;
        m_clientCount = 0;
		m_recvCount = 0;
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
            addClientToCellServer(new ClientSocket(_cSock));
            //m_vectClients.push_back(new ClientSocket(_cSock));
            //send
            //printf("�¿ͻ��˼���,sock = %d,ip = %s���ͻ����� = %d \n", _cSock, inet_ntoa(clientA.sin_addr), m_vectClients.size());
        }
        return _cSock;
    }

    //��ѯ��С�ͻ������߳�
    void addClientToCellServer(ClientSocket* pClient)
    {
        //��ѯ�ͻ������ٵ���Ϣ�����߳�
        auto pMinClient = m_vectServers[0];
        for (auto pClient : m_vectServers)
        {
            if (pClient->getClientNum() < pMinClient->getClientNum())
            {
                pMinClient = pClient;
            }
        }
        pMinClient->addClient(pClient);
		OnNetJoin(pClient);
    }
    void Start(int nCellCount)
    {
        for (int n = 0; n < nCellCount; n++)
        {
            auto ser = new CellSercer(m_sock);
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
        //�������
        if (m_sock != INVALID_SOCKET)
        {
#ifdef _WIN32
            closesocket(m_sock);
            WSACleanup();
#else
            close(m_sock);
#endif
            m_sock = INVALID_SOCKET;
        }
    }
    //���㲢���ÿ�����Ϣ����
    void time4msg()
    {
        auto t1 = m_tTime.getElaspedSecond();
        if (t1 >= 1.0)
        {
            printf("thread<%d>,time<%lf>,socket<%d>,clientNum<%d>,recvCount<%d>\n", m_vectServers.size(), t1, m_sock, m_clientCount, (int)(m_recvCount / t1));
			m_recvCount = 0;
            m_tTime.update();
        }
    }

    

    virtual void OnNetLeave(ClientSocket* pClient)
    {
        m_clientCount--;
    }
    virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header)
    {
		m_recvCount++;
    }
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		m_clientCount++;
	}
private:

};


#endif // !_EASYTCPSERVER_HPP

