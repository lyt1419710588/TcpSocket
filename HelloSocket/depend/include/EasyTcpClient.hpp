#ifndef _EASYTCPCLIENT_HPP
#define _EASYTCPCLIENT_HPP

#include "Cell.hpp"
#include "CELLNetWork.hpp"
#include "CellClient.hpp"

class EasyTcpClient
{
public:
	EasyTcpClient()
	{
		m_isConnected = false;
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
		CELLNetWork::Init();
		if (_pClient)
		{
			 CELLLog::Info("关闭之前链接，socket = %d\n",_pClient->getSocket());
			Close();
		}
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == sock)
		{
			 CELLLog::Info("socket = %d建立失败\n", _pClient->getSocket());
		}
		else
		{
			_pClient = new CellClient(sock);
			 //CELLLog::Info("socket = %d建立成功\n", _pClient->getSocket());
		}
		return 0;
	}
	//链接服务器
	int Connect(const char* ip,unsigned short port)
	{
		if (!_pClient)
		{
			//CELLLog::Info("初始化socket\n");
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
		//CELLLog::Info("<socket=%d>正在链接服务器<%s,%d>\n",_pClient->getSocket(),ip,port);
		int ret = connect(_pClient->getSocket(), (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			 CELLLog::Info("socket = %d 链接服务器%s,%d失败\n", _pClient->getSocket(), ip, port);
		}
		else
		{
			m_isConnected = true;
		    //CELLLog::Info("socket = %d 链接服务器%s,%d\n", _pClient->getSocket(), ip, port);
		}
		return ret;
	}

	//关闭socket
	void Close()
	{
		//清除环境
		if (_pClient)
		{
			delete _pClient;
			_pClient = nullptr;
		}
		m_isConnected = false;
	}

	//发送数据

	
	int _count = 0;
	//处理网络数据
	bool OnRun()
	{
		if (isRun())
		{
			SOCKET m_sock = _pClient->getSocket();
			fd_set fd_read;
			
			FD_ZERO(&fd_read);
			FD_SET(m_sock, &fd_read);
			
			fd_set fd_write;
			FD_ZERO(&fd_write);

			timeval tl = { 0,10 };
			int ret;
			if (_pClient->needWrite())
			{
				FD_SET(m_sock, &fd_write);
				ret = select(m_sock, &fd_read, &fd_write, nullptr, &tl);
			}
			else
			{
				 ret = select(m_sock, &fd_read, nullptr, nullptr, &tl);
			}
			
			//  CELLLog::Info("select ret = %d,count  = %d\n", ret, _count++);
			if (ret < 0)
			{
				CELLLog::Info("select = %d 与服务端断开链接，任务结束\n", m_sock);
				Close();
				return false;
			}
			if (FD_ISSET(m_sock, &fd_read))
			{
				if (-1 == RecvData())
				{
					CELLLog::Info("select = %d 任务结束2\n",m_sock);
					Close();
					return false;
				}
			}

			if (FD_ISSET(m_sock, &fd_write))
			{
				if (-1 == _pClient->SendDataReal())
				{
					CELLLog::Info("select = %d 任务结束2\n", m_sock);
					Close();
					return false;
				}
			/*	else
				{
					CELLLog::Info("select = %d 数据发送\n", m_sock);
				}*/
			}
			return true;
		}
		return false;
	}
	bool isRun()
	{
		return (_pClient) && m_isConnected;
	}
	//接收数据,处理粘包，拆分包
	//第二缓冲区，双缓冲
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240//缓冲区大小
#endif // !RECV_BUFF_SIZE


	//接收缓冲区
	char m_recvBUF[RECV_BUFF_SIZE] = {};
	//消息缓冲区
	char m_szMsgBUF[RECV_BUFF_SIZE * 4] = {};
	//消息缓冲区尾部位置
	int m_lastPos = 0;
	int RecvData()
	{	
		if (isRun())
		{
			//接收客户端的请求数据
			int nLen = _pClient->RecvData();

			if (nLen > 0)
			{
				while (_pClient->hasMsg())
				{
					//处理网络消息
					OnNetMsg(_pClient->front_msg());
					//移除消息队列（缓冲区）最前端的数据
					_pClient->pop_front_msg();
				}
			}
			return nLen;
		}
		return 0;
	}
	//响应网络数据
	virtual void OnNetMsg(DataHeader* header)  = 0;
	//发送数据
	int SendData(std::shared_ptr<DataHeader> data)
	{
		if (isRun())
		{
			return _pClient->SendData(data);
		}
		return 0;
	}
	int SendData(const char* pData, int nLen)
	{
		if (isRun())
		{
			return _pClient->SendData(pData, nLen);
		}
		return 0;
	}
	CellClient *getCurClient()
	{
		return _pClient;
	}
protected:
	CellClient *_pClient = nullptr;
	bool m_isConnected = false;
};


#endif // !_EASYTCPCLIENT_HPP

