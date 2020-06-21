#include "EasyTcpClient.hpp"
#include <stdio.h>
#include <thread>
#include <atomic>
#include <memory>
#include "CELLTimestamp.hpp"


class MyClient :public EasyTcpClient
{
public:
	void OnNetMsg(DataHeader* header)
	{
		//����ͻ�������
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			//���շ��������ص�����
			LoginResult *ret = (LoginResult*)header;
			// CELLLog::Info("�յ��������Ϣ��retLogin = %d�����ݳ���:%d\n", ret->result, header->dataLength);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			//���շ��������ص�����
			LogoutResult *ret = (LogoutResult*)header;
			// CELLLog::Info("�յ��������Ϣ��retLogout = %d�����ݳ���:%d\n", ret->result, header->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			//���շ��������ص�����
			NewUserJoin *ret = (NewUserJoin*)header;
			// CELLLog::Info("�յ��������Ϣ��newUerJoinIN  sock = %d�����ݳ���:%d\n", ret->sock, header->dataLength);
		}
		break;
		case CMD_ERROR:
		{
			//���շ��������ص�����
			 CELLLog::Info("�յ��������Ϣ��CMD_ERROR  sock = %d�����ݳ���:%d\n", _pClient->getSocket(), header->dataLength);
		}
		break;
		case CMD_HEART_S2C:
		{

		}
		break;
		default:
		{
			 CELLLog::Info("�յ������δ�������ݣ�  sock = %d�����ݳ���:%d\n", _pClient->getSocket(), header->dataLength);
		}
		break;
		}
	}
};
bool g_Run = true;
void cmdthread()
{
	char cmdBUF[128] = {};
	while (true)
	{
		scanf("%s", cmdBUF);
		if (0 == strcmp(cmdBUF, "exit"))
		{
			 CELLLog::Info("�˳��߳�\n");
			g_Run = false;
			break;
		}
	}
}

const int cCount = 1000;
EasyTcpClient *client[cCount];
const int tCount = 4;//�߳�����
std::atomic_int m_sendCount = 0;
std::atomic_int m_readyCount = 0;

void recvthread(int begin, int end)
{
	CELLTimestamp t;
	while (g_Run)
	{
		for (int i = begin; i < end; i++)
		{
			client[i]->OnRun();
		}
	}
}
void sendthread(int id)
{
	//ջ�ڴ�1M
	//client.initSocket();
	int nConnect = 0;
	int c = cCount / tCount;
	int begin = (id - 1) * c;
	int end = id * c;
	for (int i = begin; i < end; i++)
	{
		if (!g_Run)
		{
			nConnect = i;
			return ;
		}
		client[i] = new MyClient;
		client[i]->Connect("127.0.0.1", 4568);
	}
	//���������߳�
	std::thread mythread(recvthread, begin, end);
	mythread.detach();
	CELLLog::Info("�߳� %d����,nConnect<begin=%d><end=%d>\n", id,begin,end);
	if (!g_Run)
	{
		for (int i = begin; i < end; i++)
		{
			client[i]->Close();
			delete (client[i]);
		}
	}
	m_readyCount++;
	while (m_readyCount < tCount)
	{
		//�ȴ������߳�׼���÷�������
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);
	}
	//
	
	//
	std::shared_ptr<Login> login = std::make_shared<Login>();
	
	strcpy(login->userName, "lyt");
	strcpy(login->password, "123456");
	const int nLen = sizeof(login);
	while (g_Run)
	{
		for (int i = begin; i < end; i++)
		{
			if (SOCKET_ERROR != client[i]->SendData(login))
			{
				m_sendCount++;
			}
			else
			{
				//CELLLog::Info("�ͻ��˷�������ʧ��client = %d",client[i]->getCurClient()->getSocket());
			}
		}
		std::chrono::milliseconds t(200);
		std::this_thread::sleep_for(t);

	}
	for (int i = begin; i < end; i++)
	{
		client[i]->Close();
		delete (client[i]);
	}
	 CELLLog::Info("�߳� %d�˳�,nConnect<begin=%d><end=%d>\n", id, begin, end);
}
int  main()
{
	
	CELLLog::Instance().setLogPath("client.txt", "w");
	std::thread mythread(cmdthread);
	mythread.detach();


	for (int  i = 0; i < tCount; i++)
	{
		std::thread mythread(sendthread, i + 1);
		mythread.detach();
	}
	CELLTimestamp tcurTime;
	while (g_Run)
	{
		auto curSec = tcurTime.getElaspedSecond();
		if (curSec > 1.0)
		{
			CELLLog::Info("thread<%d>,clients<%d>,time<%lf>,sendCount<%d>\n", tCount, cCount, curSec,(int)m_sendCount);
			m_sendCount = 0;
			tcurTime.update();
		}
		CELLThread::Sleep(1);
	}
	
	CELLLog::Info("���˳�\n");
	getchar();
	return 0;
}
