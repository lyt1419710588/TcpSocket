#include "EasyTcpClient.hpp"
#include <stdio.h>
#include <thread>


void cmdthread(EasyTcpClient *pclient)
{
	char cmdBUF[128] = {};
	while (true)
	{
		scanf("%s", cmdBUF);
		if (0 == strcmp(cmdBUF, "exit"))
		{
			printf("�˳��߳�\n");
			pclient->Close();
			break;
		}
		else if (0 == strcmp(cmdBUF, "login"))
		{
			Login login;
			strcpy(login.userName, "lyt");
			strcpy(login.password, "123456");
			
			pclient->SendData(&login);
		}
		else if (0 == strcmp(cmdBUF, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "lyt");
			pclient->SendData(&logout);
		}
		else
		{
			printf("��֧�ֵ�����\n");
		}
	}
}
int  main()
{

	EasyTcpClient client;
	//client.initSocket();
	client.Connect("127.0.0.1", 4567);

	EasyTcpClient client2;
	//client.initSocket();
	client2.Connect("127.0.0.1", 4568);
	//����UI�߳�
	std::thread mythread(cmdthread, &client);
	mythread.detach();
	while (client.isRun() || client2.isRun())
	{
		client.OnRun();
		client2.OnRun();
	}
	client.Close();
	client2.Close();
	printf("���˳�\n");
	getchar();
	return 0;
}
