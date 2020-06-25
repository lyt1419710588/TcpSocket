#ifndef _EASYTCPSERVER_HPP
#define _EASYTCPSERVER_HPP

#include "Cell.hpp"
#include "CellClient.hpp"

#include "INetEvent.hpp"
#include "CellServer.hpp"
#include "CELLNetWork.hpp"
#include <stdio.h>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>

//客户端接收服务
class EasyTcpServer :public INetEvent
{
private:
	CELLThread m_thread;
    SOCKET m_sock;
    //服务列表
    std::vector<std::shared_ptr<CellServer> > m_vectServers;
    //每秒消息计时
    CELLTimestamp m_tTime;
protected:
    //客户端计数
    std::atomic_int m_clientCount;
	//接收到消息包鼠
	std::atomic_int m_recvCount;
	//消息包数
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
    //初始化
    SOCKET initSocket()
    {
		CELLNetWork::Init();

        if (INVALID_SOCKET != m_sock)
        {
            CELLLog_Info("关闭之前链接，socket = %d", m_sock);
            Close();
        }
        m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (INVALID_SOCKET == m_sock)
        {
            CELLLog_Info("socket = %d建立失败", m_sock);
        }
        else
        {
            CELLLog_Info("socket = %d建立成功", m_sock);
        }
        return m_sock;
    }
    //绑定端口号
    int Bind(const char* ip, unsigned short port)
    {
        if (INVALID_SOCKET == m_sock)
        {
            CELLLog_Info("初始化socket");
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
            CELLLog_Error("绑定端口失败,%d！", port);
        }
        else
        {
            CELLLog_Info("SUCCESS,绑定端口成功:%d！", port);
        }
        return ret;
    }
    //监听
    int Listen(int n)
    {
        //listen
        int ret = listen(m_sock, n);
        if (SOCKET_ERROR == ret)
        {
            CELLLog_Error("socket = %d监听失败！", m_sock);
        }
        else
        {
            CELLLog_Info("SUCCESS,socket = %d监听成功！", m_sock);
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
            CELLLog_Info("socket = %d错误，接受的客户端SOCKET 无效", m_sock);
        }
        else
        {
            //NewUserJoin userJoin;
            //userJoin.sock = _cSock;
            //SendDataToAll((DataHeader*)&userJoin);
			//直接make_shared无法进入对象池 
			std::shared_ptr<CellClient> c(new CellClient(_cSock));
			addClientToCellServer(c);
           // addClientToCellServer(std::make_shared<CellClient>(_cSock));
            //m_vectClients.push_back(new CellClient(_cSock));
            //send
            //CELLLog_Info("新客户端加入,sock = %d,ip = %s，客户端数 = %d ", _cSock, inet_ntoa(clientA.sin_addr), m_vectClients.size());
        }
        return _cSock;
    }

    //查询最小客户端数线程
    void addClientToCellServer(std::shared_ptr<CellClient> pClient)
    {
        //查询客户端最少得消息处理线程
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
            //注册网络事件接受对象
            ser->setEventObj(this);
            //启动消息处理服务线程
            ser->Start();
        }
		//线程
		m_thread.Start(nullptr, [this](CELLThread *pThread) {
			OnRun(pThread);
		});
    }

  
    //关闭
    void Close()
    {
		CELLLog_Info("EasyTcpServerClose begin ");
		m_thread.Close();
        //清除环境
        if (m_sock != INVALID_SOCKET)
        {
			m_vectServers.clear();
#ifdef _WIN32
            closesocket(m_sock);
#else
            close(m_sock);
#endif
            m_sock = INVALID_SOCKET;
        }
		
		CELLLog_Info("EasyTcpServerClose end ");
    }
    //计算并输出每秒的消息包数
    void time4msg()
    {
        auto t1 = m_tTime.getElaspedSecond();
        if (t1 >= 1.0)
        {
            CELLLog_Info("thread<%d>,time<%lf>,socket<%d>,clientNum<%d>,recvCount<%d>，msgCount<%d>", m_vectServers.size(), t1, m_sock,(int)m_clientCount,(int)(m_recvCount / t1),(int)(m_msgCount / t1));
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
	//处理网络信息
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


			//nfds是一个整数值，是指fd_set集合中所有描述符(socket)的范围
			//即是所有描述符最大值+1，在windows中这个参数可以写0
			timeval tl = { 0,1};
			int ret = select(m_sock + 1, &fd_read, 0, 0, &tl);
			//CELLLog_Info("select ret = %d,count  = %d",ret, _count++);
			if (ret < 0)
			{
				CELLLog_Info("EasyTcpServer::OnRun,select任务结束，退出");
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

