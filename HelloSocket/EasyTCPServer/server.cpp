#include "EasyTcpServer.hpp"
int  main()
{
	EasyTcpServer server;
	server.initSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
    while (server.isRun())
    {
		server.OnRun();
       // printf("空闲时间处理其他业务\n");
    }

	server.Close();
    printf("已退出\n");
    getchar();
    return 0;
}


