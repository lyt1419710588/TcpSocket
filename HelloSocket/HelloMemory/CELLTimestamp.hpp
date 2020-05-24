#ifndef _CELLTIMESTAMP_H
#define _CELLTIMESTAMP_H

#include <chrono>
using namespace std::chrono;
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
	//���µ�ǰʱ��
	void update()
	{
		m_begin = high_resolution_clock::now();
	}
	//��ȡ��ǰ����
	double getElaspedSecond()
	{
		return this->getElapsedTimeInMicroSec() * 0.000001;
	}
	//��ȡ��ǰ������
	double getElaspedInMillSec()
	{ 
		return this->getElapsedTimeInMicroSec() * 0.001;
	}
	//��ȡ΢��
	long long getElapsedTimeInMicroSec()
	{
		return duration_cast<microseconds>(high_resolution_clock::now() - m_begin).count();
	}
protected:
	time_point<high_resolution_clock> m_begin;

};
#endif // !_CELLTIMESTAMP_H