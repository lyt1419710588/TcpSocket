#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>

#include <vector>

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
std::vector<SOCKET> g_clents;
int processor(SOCKET _cSock)
{
	char recvBUF[4096] = {};
	//���տͻ��˵���������
	int nLen = recv(_cSock, recvBUF, sizeof(DataHeader), 0);
	DataHeader *header = (DataHeader*)recvBUF;
	if (nLen < 0)
	{
		printf("�ͻ���<socket = %d>���Ƴ��������������\n",_cSock);
		return -1;
	}

	//����ͻ�������
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		int nLen = recv(_cSock, recvBUF + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login *login = (Login*)recvBUF;
		printf("�յ�����CMD_LOGIN ���ݳ���:%d,userName = %s Password = %s\n", header->dataLength, login->userName, login->password);
		//�����ж��û��������Ƿ���ȷ
		LoginResult loginresult;
		loginresult.result = 1;
		send(_cSock, (char*)&loginresult, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGOUT:
	{
		int nLen = recv(_cSock, recvBUF + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Logout *logout = (Logout*)recvBUF;
		printf("�յ�����CMD_LOGOUT ���ݳ���:%d,userName = %s\n", header->dataLength, logout->userName);
		//�����ж��û��������Ƿ���ȷ
		LogoutResult ret;
		ret.result = 1;
		send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
	}
	break;
	default:
		DataHeader header;
		header.cmd = CMD_ERROR;
		header.dataLength = 0;
		send(_cSock, (char*)&header, sizeof(DataHeader), 0);
		break;
	}
	return 0;
}
int  main()
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(ver, &data);
	//socket
	SOCKET  _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//bind
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
	{
		printf("ERROR,�󶨶˿�ʧ�ܣ�\n");
	}
	else
	{
		printf("SUCCESS,�󶨶˿ڳɹ���\n");
	}
	//listen
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		printf("ERROR,����ʧ�ܣ�\n");
	}
	else
	{
		printf("SUCCESS,�����ɹ���\n");
	}
	
	while (true)
	{
		fd_set fd_read;
		fd_set fd_write;
		fd_set fd_except;

		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_ZERO(&fd_except);
		
		FD_SET(_sock, &fd_read);
		FD_SET(_sock, &fd_write);
		FD_SET(_sock, &fd_except);

		for (int i = (int)g_clents.size() - 1;i >= 0; i--)
		{
			FD_SET(g_clents[i], &fd_read);
		}
		//nfds��һ������ֵ����ָfd_set����������������(socket)�ķ�Χ
		//�����������������ֵ+1����windows�������������д0
		timeval tl = {1,1};
		int ret =  select(_sock + 1,&fd_read,&fd_write,&fd_except,&tl);
		if (ret < 0)
		{
			printf("select����������˳�\n");
			break;
		}
		if (FD_ISSET(_sock,&fd_read))
		{
			FD_CLR(_sock, &fd_read);
				//accept
			sockaddr_in clientA = {};
			int nClientA = sizeof(clientA);
			SOCKET _cSock = INVALID_SOCKET;
			char msgBuf[] = "Hello.I am Server!";

			_cSock = accept(_sock, (sockaddr*)&clientA, &nClientA);
			if (INVALID_SOCKET == _cSock)
			{
				printf("���󣬽��ܵĿͻ���SOCKET ��Ч\n");
			}
			else
			{
				for (int i = (int)g_clents.size() - 1; i >= 0; i--)
				{
					NewUserJoin userJoin;
					userJoin.sock = _cSock;
					send(g_clents[i], (const char*)&userJoin, sizeof(NewUserJoin), 0);
				}
				g_clents.push_back(_cSock);
				//send
				printf("�¿ͻ��˼���,sock = %d,ip = %s\n", _cSock, inet_ntoa(clientA.sin_addr));
			}
		}
		for (int i = 0; i < (int)fd_read.fd_count; i++)
		{
			if (-1 == processor(fd_read.fd_array[i]))
			{
				auto iter = std::find(g_clents.begin(), g_clents.end(), fd_read.fd_array[i]);
				if (iter != g_clents.end())
				{
					g_clents.erase(iter);
				}
			}
		}
		printf("����ʱ�䴦������ҵ��\n");
	}
	
	for (size_t i = g_clents.size() - 1; i >= 0; i--)
	{
		closesocket(g_clents[i]);
	}
	//close
	closesocket(_sock);
	WSACleanup();
	printf("���˳�\n");
	getchar();
	return 0;
}


