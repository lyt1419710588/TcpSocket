#ifndef _CELLSEMAPHORE_HPP_
#define _CELLSEMAPHORE_HPP_

#include <chrono>
#include <thread>

//��������
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

	//������ǰ�߳�
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
	//���ѵ�ǰ�߳�
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
	//��������������
	std::condition_variable m_cv;
	//�ȴ�����
	int m_Wait = 0;
	//�ͷŵȴ�����
	int m_Wakeup = 0;
};


#endif // !_CELLSEMAPHORE_HPP_

