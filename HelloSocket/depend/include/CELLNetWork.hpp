#ifndef _CELL_NETWORK_HPP_
#define _CELL_NETWORK_HPP_

#include "Cell.hpp"
class CELLNetWork
{
private:
	CELLNetWork()
	{
		//启动 Win sock 2.x
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(ver, &data);
#else
		//忽略异常信号，默认情况会导致进程终止
		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		{
			return 1;
		}
#endif //_WIN32
	}
	~CELLNetWork()
	{
#ifdef _WIN32
		WSACleanup();
		CELLLog_Info("socket 环境清理  ");
#endif // _WIN32

	}
public:
	static void Init()
	{
		static CELLNetWork obj;
	}

};


#endif // !_CELL_NETWORK_HPP_
