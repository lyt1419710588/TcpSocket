#/*include "Alloctor.h"*/
#include "Cell.hpp"
#include "EasyTcpServer.hpp"
#include "CELLMSGStream.hpp"
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
//			CELLLog_Info("退出线程");
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
			//CELLLog_Info("收到命令<socket = %d>CMD_LOGIN 数据长度:%d,userName = %s Password = %s", _cSock,header->dataLength, login->userName, login->password);
			//忽略判断用户名密码是否正确
			//LoginResult loginresult;
			//loginresult.result = 1;
			
			//pClient->SendData(&loginresult);
			std::shared_ptr<LoginResult> ret = std::make_shared<LoginResult>();
			pCellServer->addSendTask(pClient,ret);
			//if (SOCKET_ERROR == pClient->SendData(ret))
			//{
			//	//缓冲区满
			//	CELLLog_Info("<socket = %d,Send Buff Full!!!!\n>",pClient->getSocket());
			//}
		}
		break;
		case CMD_LOGOUT:
		{
			//Logout *logout = (Logout*)header;
			//CELLLog_Info("收到命令<socket = %d>CMD_LOGOUT 数据长度:%d,userName = %s", _cSock, header->dataLength, logout->userName);
			//忽略判断用户名密码是否正确
			//LogoutResult ret;
			//ret.result = 1;
			//SendData(_cSock, &ret);

			CELLRecvStream r(header);
			
			auto s1 = r.readInt8();
			auto s2 = r.readInt16();
			auto s3 = r.readInt32();
			auto s4 = r.readFloat();
			auto s5 = r.readDouble();


			char strName1[10] = {};
			r.ReadArray(strName1, 10);

			int pas1[10] = {};
			r.ReadArray(pas1, 10);

			CELLSendStream s;

			s.setNetCmd(CMD_LOGOUT_RESULT);
			s.writeInt8(15);
			s.writeInt16(15);
			s.writeInt32(15);
			s.writeFloat(15.333f);
			s.writeDouble(15.333f);
			char* strName = "server";
			s.writeArray(strName, strlen(strName));

			int pas[5] = { 1,2,3,4,5 };
			s.writeArray(pas, 5);
			s.finish();

			pClient->SendData(s.Data(), s.length());
			CELLLog_Info("myserver::OnNetMsg:Recv CMD_LOGOUT MSG ");
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
			CELLLog_Info("未定义数据，  sock = %d，数据长度:%d", pClient->getSocket(), header->dataLength);
			//DataHeader header;
			//SendData(_cSock, &header);
			break;
		}
	}
};

const char* argToStr(int argc,char* args[],int index,const char* def,const char* argName)
{
	if (index >= argc)
	{
		CELLLog_Error("argToStr,index = %d ,argc = %d,argName = %s ",index,argc, argName);
	}
	else
	{
		def = args[index];
	}
	CELLLog_Info("%s = %s",argName,def);
	return def;
}

int argToInt(int argc, char* args[], int index, int def, const char* argName)
{
	if (index >= argc)
	{
		CELLLog_Error("argToInt,index = %d ,argc = %d，argName = %s", index, argc, argName);
	}
	else
	{
		def = atoi(args[index]);
	}
	CELLLog_Info("%s = %d", argName, def);
	return def;
}
int  main(int argc,char *args[])
{
	CELLLog::Instance().setLogPath("server", "w");

	const char *strIP = argToStr(argc,args,1,"any","IP");
	uint16_t nPort = (uint16_t)argToInt(argc, args, 2, 4567, "nPort");
	int nThread = argToInt(argc, args, 3, 1, "nThread");
	int nClient = argToInt(argc, args, 4, 1, "nClient");

	if (strcmp(strIP,"any") == 0)
	{
		strIP = nullptr;
	}
	Myserver server;
	server.initSocket();
	server.Bind(strIP, nPort);
	server.Listen(5);
	server.Start(nThread);
	/*std::thread mythread(cmdthread);
	mythread.detach();*/
	char cmdBUF[128] = {};
	while (true)
	{
		scanf("%s", cmdBUF);
		if (0 == strcmp(cmdBUF, "exit"))
		{
			server.Close();
			CELLLog_Info("退出线程");
			break;
		}
		else
		{
			CELLLog_Info("不支持的命令！！！");
		}
	}

	
    CELLLog_Info("已退出");
	/*while (true)
	{
		Sleep(1);
	}*/
    getchar();
    return 0;
}


