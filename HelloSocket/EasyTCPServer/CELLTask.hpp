#ifndef _CELLTASK_H
#define _CELLTASK_H

#include <thread>
#include <mutex>
#include <list>
//��������
class CellTask
{
public:
	CellTask()
	{

	}
	//������
	virtual	~CellTask()
	{

	}

	//ִ������
	virtual void doTask()
	{

	}
private:

};

//����ִ������ķ�������
class CellTaskServer
{
private:
	//��������
	std::list<std::shared_ptr<CellTask> > m_listCellTask;
	//�������ݻ�����
	std::list<std::shared_ptr<CellTask> > m_listCellTaskBuff;
	//�ı����ݻ�������Ҫ����
	std::mutex m_mutex;
	
public:
	CellTaskServer()
	{

	}
	virtual ~CellTaskServer()
	{

	}
	//�������
	void addTask(std::shared_ptr<CellTask>& pTask)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_listCellTaskBuff.push_back(pTask);
	}
	//��������
	void Start()
	{
		//�߳�
		std::thread thread(std::mem_fn(&CellTaskServer::OnRun),this);
		thread.detach();
	}
private:
	//��������
	void OnRun()
	{
		while (true)
		{
			if (!m_listCellTaskBuff.empty())
			{
				//�ӻ�����ȡ����
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

