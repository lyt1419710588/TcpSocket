#ifndef _CELLSEMAPHORE_HPP_
#define _CELLSEMAPHORE_HPP_

#include <chrono>
#include <thread>
class CELLSemaphore
{
public:
	CELLSemaphore()
	{

	}
	~CELLSemaphore()
	{

	}

	void wait()
	{
		m_isWait = true;
		while (m_isWait)
		{
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
		}
	}

	void wakeup()
	{
		if (m_isWait)
		{
			m_isWait = false;
		}
		else
		{
			printf("CELLSemaphore wakeup error!\n");
		}
		
	}
private:

	bool m_isWait = false;
};


#endif // !_CELLSEMAPHORE_HPP_

