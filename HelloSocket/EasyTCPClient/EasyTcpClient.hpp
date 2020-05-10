#ifndef _EASYTCPCLIENT_HPP
#define _EASYTCPCLIENT_HPP

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
#include "MesssageHeader.hpp"

class EasyTcpClient
{
private:
	SOCKET m_sock;
public:
	EasyTcpClient()
	{
		m_sock = INVALID_SOCKET;
	}
	//虚析构函数
	virtual ~EasyTcpClient()
	{
		Close();
	}
	//初始化
	int initSocket()
	{
		//启动 Win sock 2.x
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(ver, &data);
#endif
		if (INVALID_SOCKET != m_sock)
		{
			printf("关闭之前链接，socket = %d\n",m_sock);
			Close();
		}
		m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == m_sock)
		{
			printf("socket = %d建立失败\n",m_sock);
		}
		else
		{
			//printf("socket = %d建立成功\n",m_sock);
		}
		return 0;
	}
	//链接服务器
	int Connect(const char* ip,unsigned short port)
	{
		if (INVALID_SOCKET == m_sock)
		{
			//printf("初始化socket\n");
			initSocket();
		}
		//链接
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		int ret = connect(m_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("socket = %d 链接服务器%s,%d失败\n",m_sock, ip, port);
		}
		else
		{
			//printf("socket = %d 链接服务器%s,%d\n",m_sock, ip, port);
		}
		return ret;
	}

	//关闭socket
	void Close()
	{
		//清除环境
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

	//发送数据

	
	int _count = 0;
	//处理网络数据
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fd_read;
			FD_ZERO(&fd_read);
			FD_SET(m_sock, &fd_read);
			timeval tl = { 0,0 };
			int ret = select(m_sock, &fd_read, NULL, NULL, &tl);
			//printf("select ret = %d,count  = %d\n", ret, _count++);
			if (ret < 0)
			{
				printf("select = %d 与服务端断开链接，任务结束\n", m_sock);
				Close();
				return false;
			}
			if (FD_ISSET(m_sock, &fd_read))
			{
				FD_CLR(m_sock, &fd_read);
				if (-1 == RecvData())
				{
					printf("select = %d 任务结束2\n",m_sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}
	bool isRun()
	{
		return m_sock != INVALID_SOCKET;
	}
	//接收数据,处理粘包，拆分包
	//第二缓冲区，双缓冲
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240//缓冲区大小
#endif // !RECV_BUFF_SIZE


	//接收缓冲区
	char m_recvBUF[RECV_BUFF_SIZE] = {};
	//消息缓冲区
	char m_szMsgBUF[RECV_BUFF_SIZE * 2] = {};
	//消息缓冲区尾部位置
	int m_lastPos = 0;
	int RecvData()
	{	
		//接收客户端的请求数据
		int nLen = recv(m_sock, m_recvBUF, RECV_BUFF_SIZE, 0);
		if (nLen < 0)
		{
			printf("与服务器断开连接！，任务结束！\n");
			return -1;
		}
		//printf("Recv len = %d\n",nLen);
		//将接收到的数据拷贝到消息缓冲区
		memcpy(m_szMsgBUF + m_lastPos, m_recvBUF, nLen);
		//消息缓冲区尾部的位置后移
		m_lastPos += nLen;
		//判断已收消息缓冲区的数据长度是否大于消息头
		while (m_lastPos >= sizeof(DataHeader))
		{
			DataHeader* header = (DataHeader*)m_szMsgBUF;
			if (m_lastPos >= header->dataLength)
			{
				//剩余未处理的消息缓冲区数据的长度
				int nSize = m_lastPos - header->dataLength;
				//处理网络消息
				OnNetMsg(header);
				//将未处理的数据前移
				memcpy(m_szMsgBUF, m_szMsgBUF + header->dataLength, nSize);
				m_lastPos = nSize;
			}
			else
			{
				break;
			}
		}
		/*DataHeader *header = (DataHeader*)recvBUF;
		if (nLen < 0)
		{
			printf("与服务器断开连接！，任务结束！\n");
			return -1;
		}
		//recv(m_sock, recvBUF + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(header);*/
		return 0;
	}
	//响应网络数据
	void OnNetMsg(DataHeader* header)
	{
		//处理客户端请求
		switch (header->cmd)
		{
			case CMD_LOGIN_RESULT:
			{
				//接收服务器返回的数据
				LoginResult *ret = (LoginResult*)header;
				//printf("收到服务端消息：retLogin = %d，数据长度:%d\n", ret->result, header->dataLength);
			}
			break;
			case CMD_LOGOUT_RESULT:
			{
				//接收服务器返回的数据
				LogoutResult *ret = (LogoutResult*)header;
				//printf("收到服务端消息：retLogout = %d，数据长度:%d\n", ret->result, header->dataLength);
			}
			break;
			case CMD_NEW_USER_JOIN:
			{
				//接收服务器返回的数据
				NewUserJoin *ret = (NewUserJoin*)header;
				//printf("收到服务端消息：newUerJoinIN  sock = %d，数据长度:%d\n", ret->sock, header->dataLength);
			}
			break;
			case CMD_ERROR:
			{
				//接收服务器返回的数据
				printf("收到服务端消息：CMD_ERROR  sock = %d，数据长度:%d\n", m_sock, header->dataLength);
			}
			break;
			default:
			{
				printf("收到服务端未定义数据，  sock = %d，数据长度:%d\n", m_sock, header->dataLength);
			}
			break;
		}
	}
	//发送数据
	int SendData(DataHeader *header,int nLen)
	{
		if (isRun() && header)
		{
			return send(m_sock, (const char*)header, nLen, 0);
		}
		return SOCKET_ERROR;
	}
private:

};


#endif // !_EASYTCPCLIENT_HPP

