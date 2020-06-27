#include "EasyTcpClient.hpp"
#include "CELLTimestamp.hpp"
#include "CELLConfig.hpp"
#include "CELLThread.hpp"
#include "CELLMSGStream.hpp"

#include <stdio.h>
#include <thread>
#include <atomic>
#include <memory>
#include <vector>


//服务端IP
const char* strIP = "127.0.0.1";
//服务端端口
uint16_t nPort = 4567;
//发送线程数量
int nThread = 1;
//客户端数量
int nClient = 10000;
//客户端每次发送消息数量
int nMsg = 100;
//写入消息到缓冲区的间隔时间
int  nSendSleep = 1000;
//工作休眠时间 
int nWorkSleep = 1;
//客户端发送缓冲区大小
int nSendBuffSize = SEND_BUFF_SIZE;
//客户端接收缓冲区大小
int nRecvBuffSize = RECV_BUFF_SIZE;



class MyClient :public EasyTcpClient
{
public:
	MyClient()
	{
		_bCheckMsgID = CELLConfig::Instance().hasKey("-checkMsgID");
	}
	void OnNetMsg(DataHeader* header)
	{
		//处理客户端请求
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			//接收服务器返回的数据
			LoginResult *login = (LoginResult*)header;
			if (_bCheckMsgID)
			{
				if (login->msgID != _nRecvMsgID)
				{
					CELLLog_Error("OnNetMsg socket<%d> msgID<%d>  _nRecveMsgID<%d> %d",_pClient->getSocket(),login->msgID,_nRecvMsgID,login->msgID - _nRecvMsgID);	
				}
				else
				{
					_nRecvMsgID++;
				}
			}
			// CELLLog_Info("收到服务端消息：retLogin = %d，数据长度:%d\n", ret->result, header->dataLength);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			//接收服务器返回的数据
			LogoutResult *ret = (LogoutResult*)header;
			// CELLLog_Info("收到服务端消息：retLogout = %d，数据长度:%d\n", ret->result, header->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			//接收服务器返回的数据
			NewUserJoin *ret = (NewUserJoin*)header;
			// CELLLog_Info("收到服务端消息：newUerJoinIN  sock = %d，数据长度:%d\n", ret->sock, header->dataLength);
		}
		break;
		case CMD_ERROR:
		{
			//接收服务器返回的数据
			 CELLLog_Info("收到服务端消息：CMD_ERROR  sock = %d，数据长度:%d\n", _pClient->getSocket(), header->dataLength);
		}
		break;
		case CMD_HEART_S2C:
		{

		}
		break;
		default:
		{
			 CELLLog_Info("收到服务端未定义数据，  sock = %d，数据长度:%d\n", _pClient->getSocket(), header->dataLength);
		}
		break;
		}
	}
public:
	int SendTest(std::shared_ptr<Login> login)
	{
		int ret = 0;
		//如果剩余发送次数大于0
		if (_nSendCount > 0)
		{
			login->msgID = _nSendMsgID;
			ret = SendData(login);
			if (SOCKET_ERROR != ret)
			{
				++_nSendMsgID; 
				//发送剩余次数减少一次
				--_nSendCount;
			}
		}
		return ret;
	}

	bool checkSend(time_t dt)
	{
		_tRestTime += dt;
		//经过nSendSleep毫秒
		if (_tRestTime >= nSendSleep)
		{
			//重置计时
			_tRestTime -= nSendSleep;
			//重置发送计数
			_nSendCount = nMsg;
		}
		return _nSendCount > 0;
	}
private:
	//接收消息id计数
	int _nRecvMsgID = 1;
	//发送消息ID计数
	int _nSendMsgID = 1;
	//发送时间计数
	time_t _tRestTime = 0;
	//发送条数计数
	int _nSendCount = 0;
	// 检查接收到的服务消息ID是否连续
	bool _bCheckMsgID = false;
};

std::atomic_int m_sendCount = 0;
std::atomic_int m_readyCount = 0;
std::atomic_int nConnect = 0;


void WorkThread(CELLThread* pThread, int id)
{
	//n个线程 id 为 1 到 n
	CELLLog_Info("thread<%d>,start",id);
	//客户端数组
	std::vector<MyClient*> clients(nClient);
	//计算本线程中客户端在clients中对应的index
	int begin = 0;
	int end = nClient;

	for (int n = begin; n < nClient; n++)
	{
		if (!pThread->isRun())
		{
			break;
		}
		clients[n] = new MyClient;
		//多线程让一下CPU
		CELLThread::Sleep(0);
	}
	for (int n = begin; n < nClient; n++)
	{
		if (!pThread->isRun())
		{
			break;
		}
		if (INVALID_SOCKET == clients[n]->initSocket(nSendBuffSize,nRecvBuffSize))
		{
			break;
		}
		if (SOCKET_ERROR == clients[n]->Connect(strIP,nPort))
		{
			break;
		}
		nConnect++;
		CELLThread::Sleep(0);
	}
	//所有链接完成
	CELLLog_Info("thread<%d>,Connect<begin=%d,end=%d,nConnect=%d>",id,begin,end,(int)nConnect);

	m_readyCount++;
	while (m_readyCount < nThread && pThread->isRun())
	{
		//等待其他线程
		CELLThread::Sleep(10);
	}
	std::shared_ptr<Login> login = std::make_shared<Login>();

	strcpy(login->userName, "lyt");
	strcpy(login->password, "123456");

	//收发数据都是通过OnRun线程
	//SendData只将数据写入发送缓冲区
	//等待select检测写入时才会发送数据


	//开始时间
	auto told = CELLTime::getNowInMilliSec();
	//当前时间
	auto tnew = told;
	//经过时间
	auto dt = tnew;

	CELLTimestamp tTime;

	while (pThread->isRun())
	{
		tnew = CELLTime::getNowInMilliSec();
		dt = tnew - told;
		told = tnew;
		//本次while(pThread->isRun())循环主要工作内容
		//代号work
		{
			int count = 0;
			//每轮每个客户端发送nMsg个数据
			for (int m  = 0; m < nMsg; m++)
			{
				//每个客户端1条1条写入数据
				for (int n = begin ; n < end ; n++)
				{ 
					if (clients[n]->isRun())
					{
						if (clients[n]->SendTest(login) > 0)
						{
							++m_sendCount;
						}
					}
				}
			}
			//sendCount+=count
			for (int n = begin; n < end; n++)
			{
				if (clients[n]->isRun())
				{
					//超时设置为0表示select状态检测后立即返回
					if (!clients[n]->OnRun(0))
					{
						//链接断开
						nConnect--;
						continue;
					}
					//检测发送计数是否需要重置
					clients[n]->checkSend(dt);
				}
			}
		}
		CELLThread::Sleep(nWorkSleep);
	}


	//关闭客户端
	for (int n = begin; n < end; n++)
	{
		clients[n]->Close();
		delete clients[n];
	}
	CELLLog_Info("thread<%d> exit",id);
	--m_readyCount;
}
int  main(int argc,char* args[])
{
	
	CELLLog::Instance().setLogPath("client", "w",false);
	CELLConfig::Instance().Init(argc, args);

	strIP = CELLConfig::Instance().getStr("strIP","127.0.0.1");
	nPort = CELLConfig::Instance().getInt("nPort", 4567);
	nThread = CELLConfig::Instance().getInt("nThread", 1);
	nClient = CELLConfig::Instance().getInt("nClient", 10000);
	nMsg = CELLConfig::Instance().getInt("nMsg", 10);
	nSendSleep = CELLConfig::Instance().getInt("nSendSleep", 100);
	nSendBuffSize = CELLConfig::Instance().getInt("nSendBuffSize", SEND_BUFF_SIZE);
	nRecvBuffSize = CELLConfig::Instance().getInt("nRecvBuffSize", RECV_BUFF_SIZE);

	//启动终端命令线程
	//用于接收运行时用户输入的命令
	CELLThread tCmd;

	tCmd.Start(nullptr, [](CELLThread *pThread) {
		while (pThread->isRun())
		{
			char cmdBUF[128] = {};
			scanf("%s", cmdBUF);
			if (0 == strcmp(cmdBUF, "exit"))
			{
				pThread->Exit();
				CELLLog_Info("退出线程");
				break;
			}
			else
			{
				CELLLog_Info("不支持的命令");
			}
		}
		
	});

	//启动模拟客户端线程
	std::vector<CELLThread*> threads;
	for (int n = 0; n < nThread; n++)
	{
		CELLThread *t = new CELLThread;
		t->Start(nullptr, [n](CELLThread* pThread) {
			WorkThread(pThread, n + 1);
		});
		threads.push_back(t);
	}

	CELLTimestamp tcurTime;
	while (tCmd.isRun())
	{
		auto curSec = tcurTime.getElaspedSecond();
		if (curSec > 1.0)
		{
			CELLLog_Info("thread<%d>,clients<%d>,connect<%d>,time<%lf>,sendCount<%d>", nThread, nClient,(int)nConnect ,curSec, (int)m_sendCount);
			m_sendCount = 0;
			tcurTime.update();
		}
		CELLThread::Sleep(1);
	}

	for (auto t : threads)
	{
		t->Close();
		delete t;
	}
	CELLLog_Info(" client 已退出");
	getchar();
	return 0;
}
