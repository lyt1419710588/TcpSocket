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
//			CELLLog::Info("�˳��߳�\n");
//			break;
//		}
//	}
//}

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

			pClient->resetDTHeart();
			Login *login = (Login*)header;
			//CELLLog::Info("�յ�����<socket = %d>CMD_LOGIN ���ݳ���:%d,userName = %s Password = %s\n", _cSock,header->dataLength, login->userName, login->password);
			//�����ж��û��������Ƿ���ȷ
			//LoginResult loginresult;
			//loginresult.result = 1;
			
			//pClient->SendData(&loginresult);
			std::shared_ptr<LoginResult> ret = std::make_shared<LoginResult>();
			pCellServer->addSendTask(pClient,ret);
			//if (SOCKET_ERROR == pClient->SendData(ret))
			//{
			//	//��������
			//	CELLLog::Info("<socket = %d,Send Buff Full!!!!\n>",pClient->getSocket());
			//}
		}
		break;
		case CMD_LOGOUT:
		{
			//Logout *logout = (Logout*)header;
			//CELLLog::Info("�յ�����<socket = %d>CMD_LOGOUT ���ݳ���:%d,userName = %s\n", _cSock, header->dataLength, logout->userName);
			//�����ж��û��������Ƿ���ȷ
			//LogoutResult ret;
			//ret.result = 1;
			//SendData(_cSock, &ret);

			CELLRecvStream r(header);
			r.getNetCmd();
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
			CELLLog::Info("δ�������ݣ�  sock = %d�����ݳ���:%d\n", pClient->getSocket(), header->dataLength);
			//DataHeader header;
			//SendData(_cSock, &header);
			break;
		}
	}
};
int  main()
{
	CELLLog::Instance().setLogPath("server.txt","w");
	Myserver server;
	server.initSocket();
	server.Bind("127.0.0.1", 4568);
	server.Listen(5);
	server.Start(1);
	/*std::thread mythread(cmdthread);
	mythread.detach();*/
	char cmdBUF[128] = {};
	while (true)
	{
		scanf("%s", cmdBUF);
		if (0 == strcmp(cmdBUF, "exit"))
		{
			server.Close();
			CELLLog::Info("�˳��߳�\n");
			break;
		}
		else
		{
			CELLLog::Info("��֧�ֵ��������\n");
		}
	}

	
    CELLLog::Info("���˳�\n");
	/*while (true)
	{
		Sleep(1);
	}*/
    getchar();
    return 0;
}


