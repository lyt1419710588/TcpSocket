#include "EasyTcpClient.hpp"
#include <stdio.h>
#include <thread>
#include <atomic>
#include "CELLTimestamp.hpp"
bool g_Run = true;
void cmdthread()
{
	char cmdBUF[128] = {};
	while (true)
	{
		scanf("%s", cmdBUF);
		if (0 == strcmp(cmdBUF, "exit"))
		{
			printf("退出线程\n");
			g_Run = false;
			break;
		}
	}
}

const int cCount = 8;
EasyTcpClient *client[cCount];
const int tCount = 4;//线程数量
std::atomic_int m_sendCount = 0;
std::atomic_int m_readyCount = 0;

void recvthread(int begin, int end)
{
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
		client[i] = new EasyTcpClient;
		client[i]->Connect("127.0.0.1", 4567);
	}
	printf("线程 %d进入,nConnect<begin=%d><end=%d>\n", id,begin,end);
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
	//建立接收线程
	std::thread mythread(recvthread,begin,end);
	mythread.detach();
	//
	Login login[1];
	for (int i = 0; i < 1; i++)
	{
		strcpy(login[i].userName, "lyt");
		strcpy(login[i].password, "123456");
	}
	const int nLen = sizeof(login);
	while (g_Run)
	{
		for (int i = begin; i < end; i++)
		{
			if (SOCKET_ERROR != client[i]->SendData(login, nLen))
			{
				m_sendCount++;
			}
		}
		/*std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);*/

	}
	for (int i = begin; i < end; i++)
	{
		client[i]->Close();
		delete (client[i]);
	}
	printf("线程 %d退出,nConnect<begin=%d><end=%d>\n", id, begin, end);
}
int  main()
{
	

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
			printf("thread<%d>,clients<%d>,time<%lf>,sendCount<%d>\n", tCount, cCount, curSec, m_sendCount);
			m_sendCount = 0;
			tcurTime.update();
		}
		Sleep(1);
	}
	
	printf("已退出\n");
	getchar();
	return 0;
}
