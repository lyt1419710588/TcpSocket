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

	//std::thread mythread(cmdthread, &client);
	//mythread.detach();

	Login login;
	strcpy(login.userName, "lyt");
	strcpy(login.password, "123456");
	while (client.isRun() )
	{
		client.OnRun();	
		client.SendData(&login);
	}
	client.Close();
	printf("���˳�\n");
	getchar();
	return 0;
}
