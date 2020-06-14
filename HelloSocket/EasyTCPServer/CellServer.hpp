#ifndef _CELLSERVER_HPP_
#define _CELLSERVER_HPP_

#include "Cell.hpp"
#include "INetEvent.hpp"
#include "CELLSemaphore.hpp"

#include <vector>
#include <map>


//网络处理收服务
class CellServer
{
public:
	CellServer(int id)
	{
		m_id = id;
		m_pInetEvent = NULL;
		m_CellTaskServer.serverID = id;
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
	
	//处理网络信息
	void  OnRun(CELLThread *pThread)
	{
		while (pThread->isRun())
		{
			m_client_change = false;
			if (m_vectClientsBuff.size() > 0)
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				for (auto iter : m_vectClientsBuff)
				{
					if (m_pInetEvent)
					{
						iter->serverID = m_id;
						m_pInetEvent->OnNetJoin(iter);
					}
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
				oldTime = CELLTime::getNowInMilliSec();
				continue;
			}
			fd_set fd_read;
			fd_set fd_write;
			//fd_set fd_Exc;
			FD_ZERO(&fd_read);
			if (m_client_change)
			{
				m_maxSock = m_vectClients.begin()->second->getSocket();
				for (auto iter : m_vectClients)
				{
					FD_SET(iter.second->getSocket(), &fd_read);
					if (m_maxSock  < iter.second->getSocket())
					{
						m_maxSock = iter.second->getSocket();
					}
				}
				memcpy(&m_fd_read_back, &fd_read, sizeof(fd_read));
				m_client_change = false;
			}
			else
			{
				memcpy(&fd_read, &m_fd_read_back, sizeof(m_fd_read_back));
			}
			memcpy(&fd_write, &m_fd_read_back, sizeof(m_fd_read_back));
			//memcpy(&fd_Exc, &m_fd_read_back, sizeof(m_fd_read_back));

			//nfds是一个整数值，是指fd_set集合中所有描述符(socket)的范围
			//即是所有描述符最大值+1，在windows中这个参数可以写0
			timeval t{ 0,1 };
			int ret = select(m_maxSock + 1, &fd_read, &fd_write, nullptr, &t);
			//printf("select ret = %d,count  = %d\n",ret, _count++);
			if (ret < 0)
			{
				printf("CELLServer%d,OnRun,selectr任务结束，ERROR\n",m_id);
				pThread->Exit();
				break;
			}

			ReadData(fd_read);
			WriteData(fd_write);
			//WriteData(fd_Exc);
			checkTime();
			//printf("CELLServer%d,fd_write=%d.fd_read=%d\n", m_id,fd_write.fd_count,fd_read.fd_count);
			/*if (fd_Exc.fd_count > 0)
			{
				printf("######fd_Exc=%d\n", fd_Exc.fd_count);
			}*/
			//return true;
		}
		printf("CellServer%d,OnRun exit\n",m_id);
		//return false;
	/*	return true;*/
	}

	
	void checkTime()
	{
		auto nowTime = CELLTime::getNowInMilliSec();
		auto dt = nowTime - oldTime;
		oldTime = nowTime;
		for (auto iter = m_vectClients.begin();iter != m_vectClients.end();)
		{
			if (iter->second->checkHeart(dt))
			{
				if (m_pInetEvent)
				{
					m_pInetEvent->OnNetLeave(iter->second);
					auto iterdel = iter++;
					m_client_change = true;
					m_vectClients.erase(iterdel);
					continue;
				}
			}
			/*iter->second->checkSendTime(dt);*/
			iter++;
		}
	}

	void OnClientLeave(std::shared_ptr<CellClient> pClient)
	{
		if (m_pInetEvent)
		{
			m_pInetEvent->OnNetLeave(pClient);
		}
		m_client_change = true;
	}
	void WriteData(fd_set &fd_write)
	{
#ifdef _WIN32
		for (int i = 0; i < fd_write.fd_count; i++)
		{
			SOCKET fd = fd_write.fd_array[i];
			auto iter = m_vectClients.find(fd);
			if (iter != m_vectClients.end())
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					iter = m_vectClients.erase(iter);	
				}
			}
		}
#else
		for (auto iter : m_vectClients)
		{
			if (FD_ISSET(iter.first, &fd_read))
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					iter = m_vectClients.erase(iter);
				}
			}
		}
#endif // _WIN32
	}
	void ReadData(fd_set &fd_read)
	{
#ifdef _WIN32
		for (int i = 0; i < fd_read.fd_count; i++)
		{
			SOCKET fd = fd_read.fd_array[i];
			auto iter = m_vectClients.find(fd);
			if (iter != m_vectClients.end())
			{
				if (-1 == RecvData(iter->second))
				{
					OnClientLeave(iter->second);
					iter = m_vectClients.erase(iter);
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
					OnClientLeave(iter->second);
					iter = m_vectClients.erase(iter);
				}
			}
		}
#endif // _WIN32
	}

	int RecvData(std::shared_ptr<CellClient>& pClient)
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
		printf("Cellserver%d Close begin\n", m_id);
		
		//清除环境
		m_CellTaskServer.Close();
		m_thread.Close();
		printf("Cellserver%d Close end\n",m_id);
	}

	void ClearClients()
	{
		m_vectClients.clear();
		m_vectClientsBuff.clear();
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
		m_thread.Start(nullptr,
			[this](CELLThread *pThread) {
			OnRun(pThread);},
			[this](CELLThread *pThread) {
				ClearClients(); });
		m_CellTaskServer.Start();
	}

	size_t getClientNum()
	{
		return m_vectClients.size() + m_vectClientsBuff.size();
	}

	void addSendTask(std::shared_ptr<CellClient> pClient, std::shared_ptr<DataHeader> ret)
	{
		//利用lambda表达式实现，避免过多的类，逻辑太复杂
		m_CellTaskServer.addTask([pClient, ret]() 
		{
			if (SOCKET_ERROR == pClient->SendData(ret))
			{

			}
		}
		);
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

	//备份客户端fd_set
	fd_set m_fd_read_back;
	//客户端列表是否变化
	bool m_client_change;
	//最大客户端sock值
	SOCKET m_maxSock;

	time_t oldTime = CELLTime::getNowInMilliSec();
	
	//线程
	CELLThread m_thread;
	//CellServerID
	int m_id = -1;
};



#endif // !_CELLSERVER_HPP_
