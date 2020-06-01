#ifndef _CELLTASK_H
#define _CELLTASK_H

#include <thread>
#include <mutex>
#include <list>
//任务类型
class CellTask
{
public:
	CellTask()
	{

	}
	//虚析构
	virtual	~CellTask()
	{

	}

	//执行任务
	virtual void doTask()
	{

	}
private:

};

//用于执行任务的服务类型
class CellTaskServer
{
private:
	//任务数据
	std::list<std::shared_ptr<CellTask> > m_listCellTask;
	//任务数据缓冲区
	std::list<std::shared_ptr<CellTask> > m_listCellTaskBuff;
	//改变数据缓冲区需要枷锁
	std::mutex m_mutex;
	
public:
	CellTaskServer()
	{

	}
	virtual ~CellTaskServer()
	{

	}
	//添加任务
	void addTask(std::shared_ptr<CellTask>& pTask)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_listCellTaskBuff.push_back(pTask);
	}
	//启动服务
	void Start()
	{
		//线程
		std::thread thread(std::mem_fn(&CellTaskServer::OnRun),this);
		thread.detach();
	}
private:
	//工作函数
	void OnRun()
	{
		while (true)
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

			for (auto iter : m_listCellTask)
			{
				iter->doTask();
			}
			m_listCellTask.clear();
		}
	}
};

#endif // !_CELLTASK_H

