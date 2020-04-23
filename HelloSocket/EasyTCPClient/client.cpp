#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <thread>
enum CMD {
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
struct DataHeader {
	short cmd;
	short dataLength;
};
struct Login :public DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char password[32];
};

struct LoginResult :public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};
struct Logout :public DataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};
struct LogoutResult :public DataHeader
{
	LogoutResult()
	{
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};

struct NewUserJoin :public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};
int processor(SOCKET _cSock)
{
	char recvBUF[4096] = {};
	//���տͻ��˵���������
	int nLen = recv(_cSock, recvBUF, sizeof(DataHeader), 0);
	DataHeader *header = (DataHeader*)recvBUF;
	if (nLen < 0)
	{
		printf("��������Ͽ����ӣ������������\n");
		return -1;
	}

	//����ͻ�������
	switch (header->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		//���շ��������ص�����
		recv(_cSock, recvBUF + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult *ret = (LoginResult*)recvBUF;
		printf("�յ��������Ϣ��retLogin = %d�����ݳ���:%d\n", ret->result, header->dataLength);
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		//���շ��������ص�����
		recv(_cSock, recvBUF + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LogoutResult *ret = (LogoutResult*)recvBUF;
		printf("�յ��������Ϣ��retLogout = %d�����ݳ���:%d\n", ret->result, header->dataLength);
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		//���շ��������ص�����
		recv(_cSock, recvBUF + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		NewUserJoin *ret = (NewUserJoin*)recvBUF;
		printf("�յ��������Ϣ��newUerJoinIN  sock = %d�����ݳ���:%d\n", ret->sock, header->dataLength);
	}
	break;
	default:
		break;
	}
	return 0;
}
bool g_bRun = true;
void cmdthread(SOCKET sock)
{
	char cmdBUF[128] = {};
	while (true)
	{
		scanf("%s", cmdBUF);
		if (0 == strcmp(cmdBUF, "exit"))
		{
			printf("�˳��߳�\n");
			g_bRun = false;
			return;
		}
		else if (0 == strcmp(cmdBUF, "login"))
		{
			Login login;
			strcpy(login.userName, "lyt");
			strcpy(login.password, "123456");
			send(sock, (const char*)&login, sizeof(Login), 0);
		}
		else if (0 == strcmp(cmdBUF, "logout"))
		{
			Logout logout;
			strcpy(logout.userName, "lyt");
			send(sock, (const char*)&logout, sizeof(Logout), 0);
		}
		else
		{
			printf("��֧�ֵ�����\n");
		}
	}
}
int  main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(ver, &data);
	//����
	SOCKET _sock = socket(AF_INET, SOCK_STREAM,0);
	if (INVALID_SOCKET == _sock)
	{
		printf("socket����ʧ��\n");
	}
	else
	{
		printf("socket�����ɹ�\n");
	}
	//����
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("socket �������Ӵ���\n");
	}
	else
	{
		printf("socket �������ӳɹ�\n");
	}

	
	std::thread mythread(cmdthread,_sock);
	mythread.detach();
	while (g_bRun)
	{
		fd_set fd_read;
		FD_ZERO(&fd_read);
		FD_SET(_sock, &fd_read);
		timeval tl = { 1,1 };
		int ret = select(_sock,&fd_read,NULL,NULL,&tl);
		if (ret < 0)
		{
			printf("select �������\n");
			break;
		}
		if (FD_ISSET(_sock, &fd_read))
		{
			FD_CLR(_sock, &fd_read);
			if (-1 == processor(_sock))
			{
				printf("select �������2\n");
				break;
			}
		}
		//printf("����ʱ�䴦������ҵ��\n");
		
		//Sleep(1000);
	}
	
	closesocket(_sock);
	WSACleanup();
	printf("���˳�\n");
	return 0;
}


