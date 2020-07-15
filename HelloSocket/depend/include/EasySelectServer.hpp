#ifndef _EASYSELECTSERVER_HPP
#define _EASYSELECTSERVER_HPP

#include "EasyTcpServer.hpp"
#include "CellSelectServer.hpp"
//客户端接收服务
class EasySelectServer :public EasyTcpServer
{
public:
	void Start(int nCellCount)
	{
		EasyTcpServer::Start<CellSelectServer>(nCellCount);
	}
private:
	//处理网络信息
	void OnRun(CELLThread *pThread)
	{
		CELLFDSet fd_read;
		while (pThread->isRun())
		{
			time4msg();
			//清理集合
			fd_read.zero();
			//将描述符加入集合
			fd_read.add(sockfd());


			//nfds是一个整数值，是指fd_set集合中所有描述符(socket)的范围
			//即是所有描述符最大值+1，在windows中这个参数可以写0
			timeval tl = { 0,1};
			int ret = select(sockfd() + 1, fd_read.fdset(), 0, 0, &tl);
			//CELLLog_Info("select ret = %d,count  = %d",ret, _count++);
			if (ret < 0)
			{
				CELLLog_Info("EasyTcpServer::OnRun,select任务结束，退出");
				pThread->Exit();
				break;
			}
			if (fd_read.has(sockfd()))
			{
				//fd_read.del(m_sock);
				Accept();
			}
		}
	}
};


#endif // !_EASYSELECTSERVER_HPP

