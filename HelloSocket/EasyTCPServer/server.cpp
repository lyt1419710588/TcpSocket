#/*include "Alloctor.h"*/
#include "Cell.hpp"
#include "EasySelectServer.hpp"
#include "CELLMSGStream.hpp"
#include "CELLConfig.hpp"

class Myserver :public EasySelectServer
{
private:
	//自定义消息标志，收到消息后将返回应答消息
	bool _bSendBack = false;
	//自定义标志，是否提示：发送缓冲区已满
	bool _bSendFull = false;
	//是否检测接收到的消息是否连续
	bool _bCheckMsgID = false;
public:
	Myserver()
	{
		_bSendBack = CELLConfig::Instance().hasKey("-sendback");
		_bSendFull = CELLConfig::Instance().hasKey("-sendfull");
		_bCheckMsgID = CELLConfig::Instance().hasKey("-checkMsgID");
	}
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
			if (_bCheckMsgID)
			{
				if (login->msgID != pClient->nRecvMsgID)
				{
					//当前消息ID和本地消息次数不匹配
					CELLLog_Error("OnNetMsg socket<%d>  msgID<%d>  nRecvMsgID<%d>   %d",
						pClient->getSocket(),login->msgID,pClient->nRecvMsgID,login->msgID - pClient->nRecvMsgID);
				}
				++pClient->nRecvMsgID;
			}
			if (_bSendBack)
			{
				std::shared_ptr<LoginResult> ret = std::make_shared<LoginResult>();
				ret->msgID = pClient->nSendMsgID;
				if (SOCKET_ERROR == pClient->SendData(ret))
				{
					//客户端发送缓冲区满了，消息没发送出去，目前直接抛弃了
					//客户端消息太多，需要考虑策略
					//正常链接，客户端不会有那么多消息
					//模拟发送测试时，是否发送频率过高
					if (_bSendFull)
					{
						CELLLog_Warring("socket<%d> SendFull",pClient->getSocket());
					}
				}
				else
				{
					++pClient->nSendMsgID;
				}
				//pCellServer->addSendTask(pClient, ret);
			}
			
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

int  main(int argc,char *args[])
{
	CELLLog::Instance().setLogPath("server", "w",false);
	CELLConfig::Instance().Init(argc, args);
	const char *strIP = CELLConfig::Instance().getStr("strIP","any");
	uint16_t nPort = CELLConfig::Instance().getInt("nPort", 4567);
	int nThread = CELLConfig::Instance().getInt("nThread", 1);

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
    return 0;
}


