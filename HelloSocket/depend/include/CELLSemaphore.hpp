#ifndef _CELLSEMAPHORE_HPP_
#define _CELLSEMAPHORE_HPP_

#include <chrono>
#include <thread>

//条件变量
#include <condition_variable>
class CELLSemaphore
{
public:
	CELLSemaphore()
	{

	}
	~CELLSemaphore()
	{

	}

	//阻塞当前线程
	void wait()
	{	
		std::unique_lock<std::mutex> lock(m_mutex);
		if (--m_Wait < 0)
		{
			m_cv.wait(lock, [this]() -> bool {
				return m_Wakeup > 0;
			});
		}
		
	}
	//唤醒当前线程
	void wakeup()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (++m_Wait <= 0)
		{
			++m_Wakeup;
			m_cv.notify_one();
		}
	}
private:
	std::mutex m_mutex;
	//阻塞的条件变量
	std::condition_variable m_cv;
	//等待计数
	int m_Wait = 0;
	//释放等待计数
	int m_Wakeup = 0;
};


#endif // !_CELLSEMAPHORE_HPP_

