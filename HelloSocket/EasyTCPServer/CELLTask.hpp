#ifndef _CELLTASK_H
#define _CELLTASK_H

#include <thread>
#include <mutex>
#include <list>
#include <functional>


#include "CELLThread.hpp"
//����ִ������ķ�������
class CellTaskServer
{
public :
	int serverID;
private:
	typedef std::function<void()> CellTask;
private:
	//��������
	std::list<CellTask> m_listCellTask;
	//�������ݻ�����
	std::list<CellTask> m_listCellTaskBuff;
	//�ı����ݻ�������Ҫ����
	std::mutex m_mutex;
	
	//�߳�
	CELLThread m_thread;
public:
	CellTaskServer()
	{

	}
	virtual ~CellTaskServer()
	{

	}
	//�������
	void addTask(CellTask pTask)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_listCellTaskBuff.push_back(pTask);
	}
	//��������
	void Start()
	{
		//�߳�
		m_thread.Start(nullptr, [this](CELLThread *pThread) {
			OnRun(pThread);
		});
	}

	//�ر�
	void Close()
	{
		
		//CELLLog::Info("CellTaskServer%d Close begin\n", serverID);
		m_thread.Close();
		//CELLLog::Info("CellTaskServer%d Close end\n", serverID);
	}
private:
	//��������
	void OnRun(CELLThread *pThread)
	{
		while (pThread->isRun())
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

			for (auto Task : m_listCellTask)
			{
				Task();
			}
			m_listCellTask.clear();
		}

		//����������е�����
		for (auto Task : m_listCellTaskBuff)
		{
			Task();
		}
		//CELLLog::Info("CellTaskServer%d,OnRun stop \n",serverID);
	}
};

#endif // !_CELLTASK_H

