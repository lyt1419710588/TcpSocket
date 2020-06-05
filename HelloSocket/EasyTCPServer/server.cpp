#include "Alloctor.h"
#include "Cell.hpp"
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

class Myserver :public EasyTcpServer
{
public:
	//�ͻ��˼���ʱ֪ͨ���ͻ����뿪�¼�
	virtual void OnNetJoin(std::shared_ptr<CellClient> pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
	}
	//�ͻ����뿪ʱ֪ͨ���ͻ����뿪�¼�
	virtual void OnNetLeave(std::shared_ptr<CellClient> pClient)
	{
		EasyTcpServer::OnNetLeave(pClient);
	}

	//Recv
	virtual void OnNetRecv(std::shared_ptr<CellClient> pClient)
	{
		EasyTcpServer::OnNetRecv(pClient);
	}
	//�ͻ��˶��յ���Ϣ��֪ͨ���߳�
	virtual void OnNetMsg(CellServer* pCellServer, std::shared_ptr<CellClient> pClient, DataHeader*  header)
	{
		EasyTcpServer::OnNetMsg(pCellServer,pClient, header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{

			Login *login = (Login*)header;
			//printf("�յ�����<socket = %d>CMD_LOGIN ���ݳ���:%d,userName = %s Password = %s\n", _cSock,header->dataLength, login->userName, login->password);
			//�����ж��û��������Ƿ���ȷ
			//LoginResult loginresult;
			//loginresult.result = 1;
			
			//pClient->SendData(&loginresult);
			std::shared_ptr<LoginResult> ret = std::make_shared<LoginResult>();
			pCellServer->addSendTask(pClient,ret);
		}
		break;
		case CMD_LOGOUT:
		{
			//Logout *logout = (Logout*)header;
			//printf("�յ�����<socket = %d>CMD_LOGOUT ���ݳ���:%d,userName = %s\n", _cSock, header->dataLength, logout->userName);
			//�����ж��û��������Ƿ���ȷ
			//LogoutResult ret;
			//ret.result = 1;
			//SendData(_cSock, &ret);
		}
		break;
		default:
			printf("δ�������ݣ�  sock = %d�����ݳ���:%d\n", pClient->getSocket(), header->dataLength);
			//DataHeader header;
			//SendData(_cSock, &header);
			break;
		}
	}
};
int  main()
{
	Myserver server;
	server.initSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	server.Start(4);
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


