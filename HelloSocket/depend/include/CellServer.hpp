#ifndef _CELLSERVER_HPP_
#define _CELLSERVER_HPP_


#include "CELLFDSet.hpp"
#include "Cell.hpp"
#include "INetEvent.hpp"
#include "CELLSemaphore.hpp"


#include <vector>
#include <map>


//网络处理收服务
class CellServer
{
public:
	CellServer()
	{
		
	}
	
	void setId(int id)
	{
		m_id = id;
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
				CELLThread::Sleep(1);
				oldTime = CELLTime::getNowInMilliSec();
				continue;
			}
			checkTime();

			if (!DoNetEvent())
			{
				pThread->Exit();
			}
			DoMsg();
		}
		CELLLog_Info("CellServer%d,OnRun exit",m_id);
		//return false;
	/*	return true;*/
	}

	virtual bool DoNetEvent() = 0;
	
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
	
	void DoMsg()
	{
		std::shared_ptr<CellClient> pClient = nullptr;
		for (auto iter : m_vectClients)
		{
			pClient = iter.second;
			while (pClient->hasMsg())
			{
				//处理网络消息
				OnNetMsg(pClient, pClient->front_msg());
				//移除消息队列（缓冲区）最前端的数据
				pClient->pop_front_msg();
			}
		}
	}
	int RecvData(std::shared_ptr<CellClient>& pClient)
	{

		//接收客户端的请求数据
		int nLen = pClient->RecvData();
		//接收网络数据事件
		m_pInetEvent->OnNetRecv(pClient);
		return nLen;
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
		
protected:
	SOCKET m_sock;
	//客户端正式队列
	std::map<SOCKET, std::shared_ptr<CellClient> > m_vectClients;
	std::vector<std::shared_ptr<CellClient> > m_vectClientsBuff;//客户端缓冲队列
	std::mutex m_mutex;//增加客户端锁,缓冲队列锁
					   //std::thread *m_pthread;
	std::shared_ptr<std::thread> m_pthread;
	//网络事件对象
	INetEvent* m_pInetEvent = nullptr;
	//接收数据
	char recvBUF[RECV_BUFF_SIZE] = {};

	CellTaskServer m_CellTaskServer;


	bool m_client_change;


	time_t oldTime = CELLTime::getNowInMilliSec();
	
	//线程
	CELLThread m_thread;

	
	//CellServerID
	int m_id = -1;
	//是否有客户端需要写数据
	bool bNeedWrite = false;
};



#endif // !_CELLSERVER_HPP_
