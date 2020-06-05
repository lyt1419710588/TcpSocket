#ifndef _CELLSERVER_HPP_
#define _CELLSERVER_HPP_

#include "Cell.hpp"
#include "INetEvent.hpp"
#include <vector>
#include <map>

//网络消息发送服务
class CellSendMsgToClientTask :public CellTask
{
private:
	std::shared_ptr<CellClient> m_pClient;
	std::shared_ptr<DataHeader> m_pHeader;
public:
	CellSendMsgToClientTask(std::shared_ptr<CellClient> pClient, std::shared_ptr<DataHeader> pHeader)
	{
		m_pClient = pClient;
		m_pHeader = pHeader;
	}

	virtual ~CellSendMsgToClientTask()
	{

	}

	virtual void doTask()
	{
		m_pClient->SendData(m_pHeader);
	}
};

//网络处理收服务
class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		m_sock = sock;
		// m_pthread = NULL;


		m_pInetEvent = NULL;
	}
	~CellServer()
	{
		Close();
		m_sock = INVALID_SOCKET;
		//delete m_pthread;
		//m_pthread = NULL;
		// delete m_pInetEvent;
		m_pInetEvent = NULL;
	}

	void setEventObj(INetEvent* pObj)
	{
		m_pInetEvent = pObj;
	}
	fd_set m_fd_read_back;
	bool m_client_change;
	SOCKET m_maxSock;
	//处理网络信息
	bool OnRun()
	{
		while (isRun())
		{
			m_client_change = false;
			if (m_vectClientsBuff.size() > 0)
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				for (auto iter : m_vectClientsBuff)
				{
					m_vectClients[iter->getSocket()] = iter;
					m_client_change = true;
				}
				m_vectClientsBuff.clear();
			}
			//如果没有需要处理得客户端就跳过
			if (m_vectClients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			fd_set fd_read;
			FD_ZERO(&fd_read);
			if (m_client_change)
			{
				m_maxSock = m_vectClients.begin()->first;
				for (auto iter : m_vectClients)
				{
					FD_SET(iter.first, &fd_read);
					if (m_maxSock  < iter.first)
					{
						m_maxSock = iter.first;
					}
				}
				memcpy(&m_fd_read_back, &fd_read, sizeof(fd_read));
				m_client_change = false;
			}
			else
			{
				memcpy(&fd_read, &m_fd_read_back, sizeof(m_fd_read_back));
			}

			//nfds是一个整数值，是指fd_set集合中所有描述符(socket)的范围
			//即是所有描述符最大值+1，在windows中这个参数可以写0
			int ret = select(m_maxSock + 1, &fd_read, nullptr, nullptr, nullptr);
			//printf("select ret = %d,count  = %d\n",ret, _count++);
			if (ret < 0)
			{
				printf("select任务结束，退出\n");
				return false;
			}
#ifdef _WIN32
			for (int i = 0; i < fd_read.fd_count; i++)
			{
				SOCKET fd = fd_read.fd_array[i];
				auto iter = m_vectClients.find(fd);
				if (iter != m_vectClients.end())
				{
					if (-1 == RecvData(iter->second))
					{
						if (m_pInetEvent)
						{
							m_pInetEvent->OnNetLeave(iter->second);
						}
						iter = m_vectClients.erase(iter);
						m_client_change = true;
					}
				}
			}
#else
			for (auto iter : m_vectClients)
			{
				if (FD_ISSET(iter.first, &fd_read))
				{
					if (-1 == RecvData(iter.second))
					{
						if (m_pInetEvent)
						{
							m_pInetEvent->OnNetLeave(iter.second);
						}
						iter = m_vectClients.erase(iter);
						m_client_change = true;
					}
				}
			}
#endif // _WIN32

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


	int RecvData(std::shared_ptr<CellClient> pClient)
	{

		//接收客户端的请求数据
		int nLen = recv(pClient->getSocket(), recvBUF, RECV_BUFF_SIZE, 0);
		m_pInetEvent->OnNetRecv(pClient);
		//printf("Recv len = %d\n", nLen);
		//DataHeader *header = (DataHeader*)recvBUF;
		if (nLen < 0)
		{
			//printf("客户端<socket = %d>已退出！，任务结束！\n", pClient->getSocket());
			return -1;
		}

		memcpy(pClient->msgBuf() + pClient->getLast(), recvBUF, nLen);
		//消息缓冲区尾部的位置后移
		pClient->setLast(pClient->getLast() + nLen);
		//判断已收消息缓冲区的数据长度是否大于消息头
		while (pClient->getLast() >= sizeof(DataHeader))
		{
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			if (pClient->getLast() >= header->dataLength)
			{
				//剩余未处理的消息缓冲区数据的长度
				int nSize = pClient->getLast() - header->dataLength;
				//处理网络消息
				OnNetMsg(pClient, header);
				//将未处理的数据前移
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
	void OnNetMsg(std::shared_ptr<CellClient> pClient, DataHeader* header)
	{
		//处理客户端请求
		m_pInetEvent->OnNetMsg(this, pClient, header);
		/*auto t1 = m_tTime.getElaspedSecond();
		if (t1 >= 1.0)
		{
		printf("time<%lf>,socket<%d>,clientNum<%d>,recvCount<%d>\n", t1, _cSock, m_vectClients.size(), m_recvCount);
		m_tTime.update();
		m_recvCount = 0;
		}*/
	}


	//关闭
	void Close()
	{
		//清除环境
		if (m_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (auto iter : m_vectClients)
			{
				closesocket(iter.first);
			}
#else
			for (auto iter : m_vectClients)
			{
				closesocket(iter.first);
			}
#endif
			m_sock = INVALID_SOCKET;
			m_vectClients.clear();
		}
	}

	void addClient(std::shared_ptr<CellClient> pClient)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		//m_mutex.lock();
		m_vectClientsBuff.push_back(pClient);
		//	m_mutex.unlock;
	}

	void Start()
	{
		// m_pthread = new std::thread(std::mem_fn(&CellServer::OnRun), this);
		m_pthread = std::make_shared<std::thread>(std::mem_fn(&CellServer::OnRun), this);
		m_CellTaskServer.Start();
	}

	size_t getClientNum()
	{
		return m_vectClients.size() + m_vectClientsBuff.size();
	}

	void addSendTask(std::shared_ptr<CellClient> pClient, std::shared_ptr<DataHeader> ret)
	{
		std::shared_ptr<CellSendMsgToClientTask> task = std::make_shared<CellSendMsgToClientTask>(pClient, ret);
		m_CellTaskServer.addTask((std::shared_ptr<CellTask>)task);
	}

private:
	SOCKET m_sock;
	//客户端正式队列
	std::map<SOCKET, std::shared_ptr<CellClient> > m_vectClients;
	std::vector<std::shared_ptr<CellClient> > m_vectClientsBuff;//客户端缓冲队列
	std::mutex m_mutex;//增加客户端锁,缓冲队列锁
					   //std::thread *m_pthread;
	std::shared_ptr<std::thread> m_pthread;
	//网络事件对象
	INetEvent* m_pInetEvent;
	//接收数据
	char recvBUF[RECV_BUFF_SIZE] = {};

	CellTaskServer m_CellTaskServer;
};



#endif // !_CELLSERVER_HPP_
