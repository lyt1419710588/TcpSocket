#ifndef _CELLEPOLLSERVER_HPP_
#define _CELLEPOLLSERVER_HPP_


#include "CELLepoll.hpp"
#include "CellServer.hpp"
//网络处理收服务
class CellEpollServer:public CellServer
{
public:
	CellEpollServer()
	{
		_ep.create(10240);
	}
	~CellEpollServer()
	{
		Close();
	}
	void OnClientLeave(std::shared_ptr<CellClient> pClient)
	{
		_ep.ctrl(EPOLL_CTL_ADD,pClient->getSocket(),EPOLLIN);
	}
	bool DoNetEvent()
	{
		for(auto iter : m_vectClients)
		{
			if(iter.second->needWrite())
			{
				_ep.ctrl(EPOLL_CTL_MOD,iter.second,EPOLLIN | EPOLLOUT);
			}
			else
			{
				_ep.ctrl(EPOLL_CTL_MOD,iter.second,EPOLLIN);
			}
		}
		int ret = _ep.wait(1);
		if(ret < 0)
		{
			CELLLog_Error("CellEpollServer::OnRun  wait error id = %d",m_id);
			return false;
		}
		else if(ret == 0)
		{
			return true;
		}

		auto events = _ep.event();
		for(int i = 0;i < ret;i++)
		{
			CellClient *temp = (CellClient*)events[i].data.ptr;
			std::shared_ptr<CellClient> pClient = std::shared_ptr<CellClient>(temp);
			if(pClient)
			{
				if(events[i].events & EPOLLIN)
				{
					if(SOCKET_ERROR == RecvData(pClient))
					{
						rmClient(pClient);
						continue;
					}
				}
				if(events[i].events & EPOLLOUT)
				{
					if(SOCKET_ERROR == pClient->SendDataReal())
					{
						rmClient(pClient);
					}
				}
			}
		}
		return true;
	}

	void rmClient(std::shared_ptr<CellClient> pClient)
	{
		auto iter = m_vectClients.find(pClient->getSocket());
		if(iter != m_vectClients.end())
		{
			m_vectClients.erase(iter);
		}
		OnClientLeave(pClient);	
	}

	void OnClientJoin(std::shared_ptr<CellClient> pClient)
	{
		_ep.ctrl(EPOLL_CTL_ADD,pClient,EPOLLIN);
	}
private:
	CEllEpoll _ep;
};



#endif // !_CELLEPOLLSERVER_HPP_
