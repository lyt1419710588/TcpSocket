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
		CELLLog_Info("CELLServer::OnRun() start--------------");
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
			checkTime();


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

			for (auto iter : m_vectClients)
			{	
				//检测需要写数据的客户端
				if (iter.second->needWrite())
				{
					bNeedWrite = true;
					FD_SET(iter.second->getSocket(), &fd_write);
				}
			}
			//memcpy(&fd_write, &m_fd_read_back, sizeof(m_fd_read_back));
			//memcpy(&fd_Exc, &m_fd_read_back, sizeof(m_fd_read_back));

			//nfds是一个整数值，是指fd_set集合中所有描述符(socket)的范围
			//即是所有描述符最大值+1，在windows中这个参数可以写0
			timeval t{ 0,1 };
			int ret = 0;
			if (bNeedWrite)
			{
				ret = select(m_maxSock + 1, &fd_read, &fd_write, nullptr, &t);
				bNeedWrite = false;
			}
			else
			{
				ret = select(m_maxSock + 1, &fd_read, nullptr, nullptr, &t);
			}
			
			//CELLLog_Info("select ret = %d,count  = %d",ret, _count++);
			if (ret < 0)
			{
				CELLLog_Info("CELLServer%d,OnRun,selectr任务结束，ERROR",m_id);
				pThread->Exit();
				break;
			}
			else if(ret == 0)
			{
				continue;
			}

			ReadData(fd_read);
			WriteData(fd_write);
			//WriteData(fd_Exc);
			
			//CELLLog_Info("CELLServer%d,fd_write=%d.fd_read=%d", m_id,fd_write.fd_count,fd_read.fd_count);
			/*if (fd_Exc.fd_count > 0)
			{
				CELLLog_Info("######fd_Exc=%d", fd_Exc.fd_count);
			}*/
			//return true;
		}
		CELLLog_Info("CellServer%d,OnRun exit",m_id);
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
			iter->second->checkSendTime(dt);
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
			if (iter->second->needWrite() && FD_ISSET(iter.first, &fd_write))
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
		int nLen = pClient->RecvData();
		
		if (nLen < 0)
		{
			CELLLog_Info("客户端<socket = %d>已退出！，任务结束！", pClient->getSocket());
			return -1;
		}
		//接收网络数据事件
		m_pInetEvent->OnNetRecv(pClient);

		//循环 判断是否有消息需要处理
		while (pClient->hasMsg())
		{
			//处理网络消息
			OnNetMsg(pClient, pClient->front_msg());
			//移除消息队列（缓冲区）最前端的数据
			pClient->pop_front_msg();
		}

		return 0;
	}
	void OnNetMsg(std::shared_ptr<CellClient> pClient, DataHeader* header)
	{
		//处理客户端请求
		m_pInetEvent->OnNetMsg(this, pClient, header);
		/*auto t1 = m_tTime.getElaspedSecond();
		if (t1 >= 1.0)
		{
		CELLLog_Info("time<%lf>,socket<%d>,clientNum<%d>,recvCount<%d>", t1, _cSock, m_vectClients.size(), m_recvCount);
		m_tTime.update();
		m_recvCount = 0;
		}*/
	}


	//关闭
	void Close()
	{
		CELLLog_Info("Cellserver%d Close begin", m_id);
		
		//清除环境
		m_CellTaskServer.Close();
		m_thread.Close();
		CELLLog_Info("Cellserver%d Close end",m_id);
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
		CELLLog_Info("CELLServer::Start(),in");
		m_CellTaskServer.Start();
		m_thread.Start(nullptr,
			[this](CELLThread *pThread) {
			OnRun(pThread);},
			[this](CELLThread *pThread) {
				ClearClients(); });
		CELLLog_Info("CELLServer::Start(),out");
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
	//是否有客户端需要写数据
	bool bNeedWrite = false;
};



#endif // !_CELLSERVER_HPP_
