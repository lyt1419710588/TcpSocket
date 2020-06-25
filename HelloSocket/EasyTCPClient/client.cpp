#include "EasyTcpClient.hpp"
#include <stdio.h>
#include <thread>
#include <atomic>
#include <memory>
#include "CELLTimestamp.hpp"


class MyClient :public EasyTcpClient
{
public:
	void OnNetMsg(DataHeader* header)
	{
		//处理客户端请求
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			//接收服务器返回的数据
			LoginResult *ret = (LoginResult*)header;
			// CELLLog_Info("收到服务端消息：retLogin = %d，数据长度:%d\n", ret->result, header->dataLength);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			//接收服务器返回的数据
			LogoutResult *ret = (LogoutResult*)header;
			// CELLLog_Info("收到服务端消息：retLogout = %d，数据长度:%d\n", ret->result, header->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			//接收服务器返回的数据
			NewUserJoin *ret = (NewUserJoin*)header;
			// CELLLog_Info("收到服务端消息：newUerJoinIN  sock = %d，数据长度:%d\n", ret->sock, header->dataLength);
		}
		break;
		case CMD_ERROR:
		{
			//接收服务器返回的数据
			 CELLLog_Info("收到服务端消息：CMD_ERROR  sock = %d，数据长度:%d\n", _pClient->getSocket(), header->dataLength);
		}
		break;
		case CMD_HEART_S2C:
		{

		}
		break;
		default:
		{
			 CELLLog_Info("收到服务端未定义数据，  sock = %d，数据长度:%d\n", _pClient->getSocket(), header->dataLength);
		}
		break;
		}
	}
};
bool g_Run = true;
void cmdthread()
{
	char cmdBUF[128] = {};
	while (true)
	{
		scanf("%s", cmdBUF);
		if (0 == strcmp(cmdBUF, "exit"))
		{
			 CELLLog_Info("退出线程\n");
			g_Run = false;
			break;
		}
	}
}

const int cCount = 1000;
EasyTcpClient *client[cCount];
const int tCount = 4;//线程数量
std::atomic_int m_sendCount = 0;
std::atomic_int m_readyCount = 0;

void recvthread(int begin, int end)
{
	CELLTimestamp t;
	while (g_Run)
	{
		for (int i = begin; i < end; i++)
		{
			client[i]->OnRun();
		}
	}
}
void sendthread(int id)
{
	//栈内存1M
	//client.initSocket();
	int nConnect = 0;
	int c = cCount / tCount;
	int begin = (id - 1) * c;
	int end = id * c;
	for (int i = begin; i < end; i++)
	{
		if (!g_Run)
		{
			nConnect = i;
			return ;
		}
		client[i] = new MyClient;
		client[i]->Connect("127.0.0.1", 4568);
	}
	//建立接收线程
	std::thread mythread(recvthread, begin, end);
	mythread.detach();
	CELLLog_Info("线程 %d进入,nConnect<begin=%d><end=%d>\n", id,begin,end);
	if (!g_Run)
	{
		for (int i = begin; i < end; i++)
		{
			client[i]->Close();
			delete (client[i]);
		}
	}
	m_readyCount++;
	while (m_readyCount < tCount)
	{
		//等待其他线程准备好发送数据
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);
	}
	//
	
	//
	std::shared_ptr<Login> login = std::make_shared<Login>();
	
	strcpy(login->userName, "lyt");
	strcpy(login->password, "123456");
	const int nLen = sizeof(login);
	while (g_Run)
	{
		for (int i = begin; i < end; i++)
		{
			if (SOCKET_ERROR != client[i]->SendData(login))
			{
				m_sendCount++;
			}
			else
			{
				//CELLLog_Info("客户端发送数据失败client = %d",client[i]->getCurClient()->getSocket());
			}
		}
		std::chrono::milliseconds t(200);
		std::this_thread::sleep_for(t);

	}
	for (int i = begin; i < end; i++)
	{
		client[i]->Close();
		delete (client[i]);
	}
	 CELLLog_Info("线程 %d退出,nConnect<begin=%d><end=%d>\n", id, begin, end);
}
int  main()
{
	
	CELLLog::Instance().setLogPath("client", "w");
	std::thread mythread(cmdthread);
	mythread.detach();


	for (int  i = 0; i < tCount; i++)
	{
		std::thread mythread(sendthread, i + 1);
		mythread.detach();
	}
	CELLTimestamp tcurTime;
	while (g_Run)
	{
		auto curSec = tcurTime.getElaspedSecond();
		if (curSec > 1.0)
		{
			CELLLog_Info("thread<%d>,clients<%d>,time<%lf>,sendCount<%d>\n", tCount, cCount, curSec,(int)m_sendCount);
			m_sendCount = 0;
			tcurTime.update();
		}
		CELLThread::Sleep(1);
	}
	
	CELLLog_Info("已退出\n");
	getchar();
	return 0;
}
