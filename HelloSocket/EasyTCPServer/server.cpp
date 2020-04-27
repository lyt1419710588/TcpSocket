#include "EasyTcpServer.hpp"
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
			g_Run = false;
			printf("�˳��߳�\n");
			break;
		}
	}
}
int  main()
{
	EasyTcpServer server;
	server.initSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	std::thread mythread(cmdthread);
	mythread.detach();
    while (g_Run)
    {
		server.OnRun();
       // printf("����ʱ�䴦������ҵ��\n");
    }

	server.Close();
    printf("���˳�\n");
    getchar();
    return 0;
}


