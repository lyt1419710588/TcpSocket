#ifndef _CELLTASK_H
#define _CELLTASK_H

#include <thread>
#include <mutex>
#include <list>
#include <functional>

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
	
	//ִ��
	bool m_isRun = false;
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
		m_isRun = true;
		std::thread thread(std::mem_fn(&CellTaskServer::OnRun),this);
		thread.detach();
	}

	//�ر�
	void Close()
	{
		printf("CellTaskServer Close,serverID = %d \ n", serverID);
		m_isRun = false;
	}
private:
	//��������
	void OnRun()
	{
		while (m_isRun)
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
		printf("CellTaskServer,OnRun stop \ n");
	}
};

#endif // !_CELLTASK_H

