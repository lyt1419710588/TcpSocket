#ifndef _CELL_NETWORK_HPP_
#define _CELL_NETWORK_HPP_

#include "Cell.hpp"
class CELLNetWork
{
private:
	CELLNetWork()
	{
		//���� Win sock 2.x
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(ver, &data);
#else
		//�����쳣�źţ�Ĭ������ᵼ�½�����ֹ
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
		CELLLog_Info("socket ��������  ");
#endif // _WIN32

	}
public:
	static void Init()
	{
		static CELLNetWork obj;
	}

};


#endif // !_CELL_NETWORK_HPP_
