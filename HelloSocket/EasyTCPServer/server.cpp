#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifdef _WIN32
#include <Windows.h>
#include <WinSock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#define SOCKET  int
#define INVALID_SOCKET (SOCKET)(0)
#define SOCKET_ERROR (-1)
#endif// _WIN32
#include <stdio.h>

#include <vector>
#ifdef _WIN32
    #pragma comment(lib,"ws2_32.lib")
#endif // _WIN32
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
    //接收客户端的请求数据
    int nLen = recv(_cSock, recvBUF, sizeof(DataHeader), 0);
    DataHeader *header = (DataHeader*)recvBUF;
    if (nLen < 0)
    {
        printf("客户端<socket = %d>已推出！，任务结束！\n", _cSock);
        return -1;
    }

    //处理客户端请求
    switch (header->cmd)
    {
    case CMD_LOGIN:
    {
        int nLen = recv(_cSock, recvBUF + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
        Login *login = (Login*)recvBUF;
        printf("收到命令CMD_LOGIN 数据长度:%d,userName = %s Password = %s\n", header->dataLength, login->userName, login->password);
        //忽略判断用户名密码是否正确
        LoginResult loginresult;
        loginresult.result = 1;
        send(_cSock, (char*)&loginresult, sizeof(LoginResult), 0);
    }
    break;
    case CMD_LOGOUT:
    {
        int nLen = recv(_cSock, recvBUF + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
        Logout *logout = (Logout*)recvBUF;
        printf("收到命令CMD_LOGOUT 数据长度:%d,userName = %s\n", header->dataLength, logout->userName);
        //忽略判断用户名密码是否正确
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
#ifdef _WIN32
    WORD ver = MAKEWORD(2, 2);
    WSADATA data;
    WSAStartup(ver, &data);
#endif // _WIN32

   
    //socket
    SOCKET  _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //bind
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
#ifdef _WIN32
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#else
    _sin.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif
    if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
    {
        printf("ERROR,绑定端口失败！\n");
    }
    else
    {
        printf("SUCCESS,绑定端口成功！\n");
    }
    //listen
    if (SOCKET_ERROR == listen(_sock, 5))
    {
        printf("ERROR,监听失败！\n");
    }
    else
    {
        printf("SUCCESS,监听成功！\n");
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

		SOCKET maxSoc = _sock;
        for (int i = (int)g_clents.size() - 1; i >= 0; i--)
        {
            FD_SET(g_clents[i], &fd_read);
			if (maxSoc  < g_clents[i])
			{
				maxSoc = g_clents[i];
			}
        }
        //nfds是一个整数值，是指fd_set集合中所有描述符(socket)的范围
        //即是所有描述符最大值+1，在windows中这个参数可以写0
        timeval tl = { 1,1 };
        int ret = select(maxSoc + 1, &fd_read, &fd_write, &fd_except, &tl);
        if (ret < 0)
        {
            printf("select任务结束，退出\n");
            break;
        }
        if (FD_ISSET(_sock, &fd_read))
        {
            FD_CLR(_sock, &fd_read);
            //accept
            sockaddr_in clientA = {};
#ifdef _WIN32
            int nClientA = sizeof(clientA);
#else
            socklen_t nClientA = sizeof(clientA);
#endif // _WIN32


            SOCKET _cSock = INVALID_SOCKET;
            char msgBuf[] = "Hello.I am Server!";

            _cSock = accept(_sock, (sockaddr*)&clientA, &nClientA);
            if (INVALID_SOCKET == _cSock)
            {
                printf("错误，接受的客户端SOCKET 无效\n");
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
                printf("新客户端加入,sock = %d,ip = %s\n", _cSock, inet_ntoa(clientA.sin_addr));
            }
        }
#ifdef _WIN32
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
#else
        auto it = g_clents.begin();
        while (it != g_clents.end())
        {
            if (FD_ISSET(*it, &fd_read))
            {
                if (-1 == processor(*it))
                {
                    auto itdel = it;
                    it++;
                    g_clents.erase(itdel);
                }
                else
                {
                    it++;
                }
            }
        }
#endif // _WIN32

       
        printf("空闲时间处理其他业务\n");
    }

#ifdef _WIN32
    for (size_t i = g_clents.size() - 1; i >= 0; i--)
    {
        closesocket(g_clents[i]);
    }
#else
    for (size_t i = g_clents.size() - 1; i >= 0; i--)
    {
        close(g_clents[i]);
    }
#endif
    //close
#ifdef _WIN32
    closesocket(_sock);
    WSACleanup();
#else
    close(_sock);
#endif
    printf("已退出\n");
    getchar();
    return 0;
}


