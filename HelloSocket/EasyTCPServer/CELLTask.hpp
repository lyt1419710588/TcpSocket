#ifndef _CELLTASK_H
#define _CELLTASK_H

#include <thread>
#include <mutex>
#include <list>
#include <functional>
#include "CELLSemaphore.hpp"
//用于执行任务的服务类型
class CellTaskServer
{
public :
	int serverID;
private:
	typedef std::function<void()> CellTask;
private:
	//任务数据
	std::list<CellTask> m_listCellTask;
	//任务数据缓冲区
	std::list<CellTask> m_listCellTaskBuff;
	//改变数据缓冲区需要枷锁
	std::mutex m_mutex;
	
	//执行
	bool m_isRun = false;

	CELLSemaphore m_sem;
public:
	CellTaskServer()
	{

	}
	virtual ~CellTaskServer()
	{

	}
	//添加任务
	void addTask(CellTask pTask)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_listCellTaskBuff.push_back(pTask);
	}
	//启动服务
	void Start()
	{
		//线程
		m_isRun = true;
		std::thread thread(std::mem_fn(&CellTaskServer::OnRun),this);
		thread.detach();
	}

	//关闭
	void Close()
	{
		if (m_isRun)
		{
			printf("CellTaskServer%d Close begin\n", serverID);
			m_isRun = false;
			m_sem.wait();
			printf("CellTaskServer%d Close end\n", serverID);
		}
	}
private:
	//工作函数
	void OnRun()
	{
		while (m_isRun)
		{
			if (!m_listCellTaskBuff.empty())
			{
				//从缓冲区取数据
				std::lock_guard<std::mutex> lock(m_mutex);
				for (auto pTask : m_listCellTaskBuff)
				{
					m_listCellTask.push_back(pTask);
				}
				m_listCellTaskBuff.clear();
			}
			if (m_listCellTask.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			for (auto Task : m_listCellTask)
			{
				Task();
			}
			m_listCellTask.clear();
		}
		printf("CellTaskServer%d,OnRun stop \n",serverID);
		m_sem.wakeup();
	}
};

#endif // !_CELLTASK_H

