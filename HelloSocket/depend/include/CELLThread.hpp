#ifndef _CELLTHREAD_HPP_
#define _CELLTHREAD_HPP_

#include "CELLSemaphore.hpp"

class CELLThread
{
public:
	static void Sleep(time_t dt)
	{
		std::chrono::milliseconds t(dt);
		std::this_thread::sleep_for(t);
	}
private:
	typedef std::function<void(CELLThread*)>  EventCall;
public:
	void Start(EventCall onCreate = nullptr, EventCall onRun = nullptr, EventCall onDestory = nullptr)
	{	
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_bRun)
		{
			m_bRun = true;
			m_OnCreate = onCreate;
			m_OnRun = onRun;
			m_OnDestory = onDestory;

			std::thread t(std::mem_fun(&CELLThread::OnWork), this);
			t.detach();

		}
		
	}
	void Close()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_bRun)
		{
			m_bRun = false;
			m_sem.wait();
		}
	}

	//在工作函数中退出不需要等待
	//不需要使用信号量阻塞等待
	//如果使用信号量，会造成死锁
	void Exit()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_bRun)
		{
			m_bRun = false;
		}
	}
	bool isRun()
	{
		return m_bRun;
	}
protected:
	void OnWork()
	{
		if (m_OnCreate)
		{
			m_OnCreate(this);
		}
		if (m_OnRun)
		{
			m_OnRun(this);
		}
		if (m_OnDestory)
		{
			m_OnDestory(this);
		}
		m_sem.wakeup();
	}
private:
	EventCall m_OnCreate;
	EventCall m_OnRun;
	EventCall m_OnDestory;

	CELLSemaphore m_sem;
	//多线程中使用该线程类枷锁
	std::mutex m_mutex;
	bool m_bRun;
};
#endif // !_CELLTHREAD_HPP_
