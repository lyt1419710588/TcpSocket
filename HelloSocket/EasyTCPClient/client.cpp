#include "EasyTcpClient.hpp"
#include <stdio.h>
#include <thread>

bool g_Run = true;
void cmdthread()
{
	char cmdBUF[128] = {};
	while (true)
	{
		scanf("%s", cmdBUF);
		if (0 == strcmp(cmdBUF, "exit"))
		{
			printf("�˳��߳�\n");
			g_Run = false;
			break;
		}
	}
}

const int cCount = 20;
EasyTcpClient *client[cCount];
const int tCount = 4;//�߳�����
void sendthread(int id)
{
	
	//ջ�ڴ�1M
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
	printf("�߳� %d����,nConnect<begin=%d><end=%d>\n", id,begin,end);
	if (!g_Run)
	{
		for (int i = begin; i < end; i++)
		{
			client[i]->Close();
			delete (client[i]);
		}
	}

	std::chrono::milliseconds t(3000);
	std::this_thread::sleep_for(t);
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
			client[i]->SendData(login, nLen);
		//	client[i]->OnRun();
		}

	}
	for (int i = begin; i < end; i++)
	{
		client[i]->Close();
		delete (client[i]);
	}
	printf("�߳� %d�˳�,nConnect<begin=%d><end=%d>\n", id, begin, end);
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
	while (g_Run)
	{
		Sleep(0);
	}
	
	printf("���˳�\n");
	getchar();
	return 0;
}
