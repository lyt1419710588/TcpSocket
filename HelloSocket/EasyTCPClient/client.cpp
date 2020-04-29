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
			printf("退出线程\n");
			g_Run = false;
			break;
		}
	}
}
int  main()
{
	const int cCount = FD_SETSIZE - 1;
	EasyTcpClient *client[cCount];
	//栈内存1M
	//client.initSocket();
	for (int  i = 0; i < cCount; i++)
	{
		client[i] = new EasyTcpClient;
		client[i]->Connect("127.0.0.1", 4567); 
	}
	

	for (int i = 0; i < cCount; i++)
	{
		client[i] = new EasyTcpClient;
		client[i]->Connect("127.0.0.1", 4567);
	}
	std::thread mythread(cmdthread);
	mythread.detach();

	Login login;
	strcpy(login.userName, "lyt");
	strcpy(login.password, "123456");
	while (g_Run)
	{
		for (int i = 0; i < cCount; i++)
		{
			client[i]->SendData(&login);
			client[i]->OnRun();
		}
		
	}
	for (int i = 0; i < cCount; i++)
	{
		client[i]->Close();
		delete (client[i]);
	}
	
	printf("已退出\n");
	getchar();
	return 0;
}
