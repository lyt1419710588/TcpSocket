#ifndef _EASYTCPSERVER_HPP
#define _EASYTCPSERVER_HPP
#ifdef _WIN32
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
class EasyTcpServer
{
private:
	SOCKET m_sock;
	std::vector<SOCKET> m_vectClients;
public:
	EasyTcpServer()
	{
		m_sock = INVALID_SOCKET; 
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	//初始化
	SOCKET initSocket()
	{
		//启动 Win sock 2.x
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(ver, &data);
#endif
		if (INVALID_SOCKET != m_sock)
		{
			printf("关闭之前链接，socket = %d\n", m_sock);
			Close();
		}
		m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == m_sock)
		{
			printf("socket = %d建立失败\n", m_sock);
		}
		else
		{
			printf("socket = %d建立成功\n", m_sock);
		}
		return m_sock;
	}
	//绑定端口号
	int Bind(const char* ip,unsigned short port)
	{
		if (INVALID_SOCKET == m_sock)
		{
			printf("初始化socket\n");
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
			sin.sin_addr.s_addr =  INADDR_ANY;
		}
#endif
		int ret = bind(m_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
		{
			printf("ERROR,绑定端口失败,%d！\n",port);
		}
		else
		{
			printf("SUCCESS,绑定端口成功:%d！\n",port);
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
			printf("ERROR,socket = %d监听失败！\n",m_sock);
		}
		else
		{
			printf("SUCCESS,socket = %d监听成功！\n",m_sock);
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
			printf("socket = %d错误，接受的客户端SOCKET 无效\n",m_sock);
		}
		else
		{
			NewUserJoin userJoin;
			userJoin.sock = _cSock;
			SendDataToAll((DataHeader*)&userJoin);
			m_vectClients.push_back(_cSock);
			//send
			printf("新客户端加入,sock = %d,ip = %s\n", _cSock, inet_ntoa(clientA.sin_addr));
		}
		return _cSock;
	}
	//关闭
	void Close()
	{
		//清除环境
		if (m_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (size_t i = m_vectClients.size() - 1; i >= 0; i--)
			{
				closesocket(m_vectClients[i]);
			}
			closesocket(m_sock);
			WSACleanup();
#else
			for (size_t i = m_vectClients.size() - 1; i >= 0; i--)
			{
				close(m_vectClients[i]);
			}
			close(m_sock);
#endif
			m_sock = INVALID_SOCKET;
		}
	}

	//
	bool isRun()
	{
		return m_sock != INVALID_SOCKET;
	}
	int _count = 0;
	//处理网络信息
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fd_read;
			fd_set fd_write;
			fd_set fd_except;

			FD_ZERO(&fd_read);
			FD_ZERO(&fd_write);
			FD_ZERO(&fd_except);

			FD_SET(m_sock, &fd_read);
			FD_SET(m_sock, &fd_write);
			FD_SET(m_sock, &fd_except);

			SOCKET maxSoc = m_sock;
			for (int i = (int)m_vectClients.size() - 1; i >= 0; i--)
			{
				FD_SET(m_vectClients[i], &fd_read);
				if (maxSoc  < m_vectClients[i])
				{
					maxSoc = m_vectClients[i];
				}
			}
			//nfds是一个整数值，是指fd_set集合中所有描述符(socket)的范围
			//即是所有描述符最大值+1，在windows中这个参数可以写0
			timeval tl = { 1,1 };
			int ret = select(maxSoc + 1, &fd_read, &fd_write, &fd_except, &tl);
			//printf("select ret = %d,count  = %d\n",ret, _count++);
			if (ret < 0)
			{
				printf("select任务结束，退出\n");
				return false;
			}
			if (FD_ISSET(m_sock, &fd_read))
			{
				FD_CLR(m_sock, &fd_read);
				Accept();
			}

			auto it = m_vectClients.begin();
			while (it != m_vectClients.end())
			{
				if (FD_ISSET(*it, &fd_read))
				{
					if (-1 == RecvData(*it))
					{
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
			return true;
		}
		return false;
		
	}
	//接收数据
	char recvBUF[409600] = {};
	int RecvData(SOCKET _cSock)
	{
		
		//接收客户端的请求数据
		int nLen = recv(_cSock, recvBUF, 409600, 0);
		//printf("Recv len = %d\n", nLen);
		//DataHeader *header = (DataHeader*)recvBUF;
		if (nLen < 0)
		{
			printf("客户端<socket = %d>已推出！，任务结束！\n", _cSock);
			return -1;
		}
		LoginResult ret;
		SendData(_cSock, &ret);
		/* recv(_cSock, recvBUF + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		 OnNetMsg(_cSock, header);*/
		return 0;
	}
	void OnNetMsg(SOCKET _cSock,DataHeader* header)
	{
		//处理客户端请求
		switch (header->cmd)
		{
			case CMD_LOGIN:
			{
			
				Login *login = (Login*)header;
				printf("收到命令CMD_LOGIN 数据长度:%d,userName = %s Password = %s\n", header->dataLength, login->userName, login->password);
				//忽略判断用户名密码是否正确
				LoginResult loginresult;
				loginresult.result = 1;
				send(_cSock, (char*)&loginresult, sizeof(LoginResult), 0);
			}
			break;
			case CMD_LOGOUT:
			{
				Logout *logout = (Logout*)header;
				printf("收到命令CMD_LOGOUT 数据长度:%d,userName = %s\n", header->dataLength, logout->userName);
				//忽略判断用户名密码是否正确
				LogoutResult ret;
				ret.result = 1;
				send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
			}
			break;
			default:
				DataHeader header;
				header.cmd = CMD_ERROR;
				header.dataLength = 0;
				send(_cSock, (char*)&header, sizeof(DataHeader), 0);
				break;
		}
	}	
	int SendData(SOCKET _cSock,DataHeader *header)
	{
		if (isRun() && header)
		{
			return send(_cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	void SendDataToAll(DataHeader *header)
	{
		for (int i = (int)m_vectClients.size() - 1; i >= 0; i--)
		{
			SendData(m_vectClients[i], header);
		}
	}
private:

};


#endif // !_EASYTCPSERVER_HPP

