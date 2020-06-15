#/*include "Alloctor.h"*/
#include "Cell.hpp"
#include "EasyTcpServer.hpp"
#include <stdio.h>
#include <thread>

//bool g_Run = true;
//void cmdthread()
//{
//	char cmdBUF[128] = {};
//	while (true)
//	{
//		scanf("%s", cmdBUF);
//		if (0 == strcmp(cmdBUF, "exit"))
//		{
//			g_Run = false;
//			printf("退出线程\n");
//			break;
//		}
//	}
//}

class Myserver :public EasyTcpServer
{
public:
	//客户端加入时通知，客户端离开事件
	virtual void OnNetJoin(std::shared_ptr<CellClient> pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
	}
	//客户端离开时通知，客户端离开事件
	virtual void OnNetLeave(std::shared_ptr<CellClient> pClient)
	{
		EasyTcpServer::OnNetLeave(pClient);
	}

	//Recv
	virtual void OnNetRecv(std::shared_ptr<CellClient> pClient)
	{
		EasyTcpServer::OnNetRecv(pClient);
	}
	//客户端端收到消息后通知主线程
	virtual void OnNetMsg(CellServer* pCellServer, std::shared_ptr<CellClient> pClient, DataHeader*  header)
	{
		EasyTcpServer::OnNetMsg(pCellServer,pClient, header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{

			pClient->resetDTHeart();
			Login *login = (Login*)header;
			//printf("收到命令<socket = %d>CMD_LOGIN 数据长度:%d,userName = %s Password = %s\n", _cSock,header->dataLength, login->userName, login->password);
			//忽略判断用户名密码是否正确
			//LoginResult loginresult;
			//loginresult.result = 1;
			
			//pClient->SendData(&loginresult);
			std::shared_ptr<LoginResult> ret = std::make_shared<LoginResult>();
			pCellServer->addSendTask(pClient,ret);
			//if (SOCKET_ERROR == pClient->SendData(ret))
			//{
			//	//缓冲区满
			//	printf("<socket = %d,Send Buff Full!!!!\n>",pClient->getSocket());
			//}
		}
		break;
		case CMD_LOGOUT:
		{
			//Logout *logout = (Logout*)header;
			//printf("收到命令<socket = %d>CMD_LOGOUT 数据长度:%d,userName = %s\n", _cSock, header->dataLength, logout->userName);
			//忽略判断用户名密码是否正确
			//LogoutResult ret;
			//ret.result = 1;
			//SendData(_cSock, &ret);
		}
		break;
		case CMD_HEART_C2S:
		{
			pClient->resetDTHeart();
			std::shared_ptr<netmsg_s2c_Heart> ret = std::make_shared<netmsg_s2c_Heart>();
			pCellServer->addSendTask(pClient, ret);
		}
		break;
		default:
			printf("未定义数据，  sock = %d，数据长度:%d\n", pClient->getSocket(), header->dataLength);
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
	/*std::thread mythread(cmdthread);
	mythread.detach();*/
	char cmdBUF[128] = {};
	while (true)
	{
		scanf("%s", cmdBUF);
		if (0 == strcmp(cmdBUF, "exit"))
		{
			server.Close();
			printf("退出线程\n");
			break;
		}
		else
		{
			printf("不支持的命令！！！\n");
		}
	}

	
    printf("已退出\n");
	while (true)
	{
		Sleep(1);
	}
    getchar();
    return 0;
}


