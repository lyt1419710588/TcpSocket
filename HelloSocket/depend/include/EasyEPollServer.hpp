#ifndef _EASYSEPOLLSERVER_HPP
#define _EASYSEPOLLSERVER_HPP

#include "EasyTcpServer.hpp"
#include "CellEpollServer.hpp"
#include "CELLepoll.hpp"
//客户端接收服务
class EasyEpollServer :public EasyTcpServer
{
public:
	void Start(int nCellCount)
	{
		EasyTcpServer::Start<CellEpollServer>(nCellCount);
	}
private:
	//处理网络信息
	void OnRun(CELLThread *pThread)
	{
		 CEllEpoll ep;
		 ep.create(1); 
   		 ep.ctrl(EPOLL_CTL_ADD,m_sock,EPOLLIN);

		while (pThread->isRun())
		{
			time4msg();
			
			int ret = ep.wait(1);
			//CELLLog_Info("select ret = %d,count  = %d",ret, _count++);
			if (ret < 0)
			{
				CELLLog_Info("EasyEpollServer::OnRun,epoll任务结束，退出");
				pThread->Exit();
				break;
			}

			auto events = ep.event();
			for(int i = 0; i < ret;i++)
			{
				if(events[i].data.fd == sockfd())
				{
					if(events[i].events & EPOLLIN)
					{
						Accept();
					}
				}
			}
		}
	}
};


#endif // !_EASYSEPOLLSERVER_HPP

