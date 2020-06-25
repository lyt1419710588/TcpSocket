#ifndef _CELLTIMESTAMP_H
#define _CELLTIMESTAMP_H

#include <chrono>
using namespace std::chrono;

class CELLTime
{
public:
	//获取当前时间戳
	static time_t getNowInMilliSec()
	{
		return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	}
};
class CELLTimestamp
{
public:
	CELLTimestamp()
	{
		update();
	}
	~CELLTimestamp()
	{

	}
	//更新当前时间
	void update()
	{
		m_begin = high_resolution_clock::now();
	}
	//获取当前秒数
	double getElaspedSecond()
	{
		return this->getElapsedTimeInMicroSec() * 0.000001;
	}
	//获取当前毫秒数
	double getElaspedInMillSec()
	{ 
		return this->getElapsedTimeInMicroSec() * 0.001;
	}
	//获取微妙
	long long getElapsedTimeInMicroSec()
	{
		return duration_cast<microseconds>(high_resolution_clock::now() - m_begin).count();
	}
protected:
	time_point<high_resolution_clock> m_begin;

};
#endif // !_CELLTIMESTAMP_H