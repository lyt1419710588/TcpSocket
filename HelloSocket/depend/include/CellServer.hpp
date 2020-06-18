#ifndef _CELLSERVER_HPP_
#define _CELLSERVER_HPP_

#include "Cell.hpp"
#include "INetEvent.hpp"
#include "CELLSemaphore.hpp"

#include <vector>
#include <map>


//���紦���շ���
class CellServer
{
public:
	CellServer(int id)
	{
		m_id = id;
		m_pInetEvent = NULL;
		m_CellTaskServer.serverID = id;
	}
	~CellServer()
	{
		Close();
		m_sock = INVALID_SOCKET;
		//delete m_pthread;
		//m_pthread = NULL;
		// delete m_pInetEvent;
		m_pInetEvent = NULL;
	}

	void setEventObj(INetEvent* pObj)
	{
		m_pInetEvent = pObj;
	}
	
	//����������Ϣ
	void  OnRun(CELLThread *pThread)
	{
		while (pThread->isRun())
		{
			m_client_change = false;
			if (m_vectClientsBuff.size() > 0)
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				for (auto iter : m_vectClientsBuff)
				{
					if (m_pInetEvent)
					{
						iter->serverID = m_id;
						m_pInetEvent->OnNetJoin(iter);
					}
					m_vectClients[iter->getSocket()] = iter;
					m_client_change = true;
				}
				m_vectClientsBuff.clear();
			}
			//���û����Ҫ����ÿͻ��˾�����
			if (m_vectClients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				oldTime = CELLTime::getNowInMilliSec();
				continue;
			}
			checkTime();
			fd_set fd_read;
			fd_set fd_write;
			//fd_set fd_Exc;
			FD_ZERO(&fd_read);
			if (m_client_change)
			{
				m_maxSock = m_vectClients.begin()->second->getSocket();
				for (auto iter : m_vectClients)
				{
					FD_SET(iter.second->getSocket(), &fd_read);
					if (m_maxSock  < iter.second->getSocket())
					{
						m_maxSock = iter.second->getSocket();
					}
				}
				memcpy(&m_fd_read_back, &fd_read, sizeof(fd_read));
				m_client_change = false;
			}
			else
			{
				memcpy(&fd_read, &m_fd_read_back, sizeof(m_fd_read_back));
			}

			for (auto iter : m_vectClients)
			{	
				if (iter.second->needWrite())
				{
					FD_SET(iter.second->getSocket(), &fd_write);
					m_maxSock = iter.second->getSocket();
				}
			}
			//memcpy(&fd_write, &m_fd_read_back, sizeof(m_fd_read_back));
			//memcpy(&fd_Exc, &m_fd_read_back, sizeof(m_fd_read_back));

			//nfds��һ������ֵ����ָfd_set����������������(socket)�ķ�Χ
			//�����������������ֵ+1����windows�������������д0
			timeval t{ 0,1 };
			int ret = select(m_maxSock + 1, &fd_read, &fd_write, nullptr, &t);
			//CELLLog::Info("select ret = %d,count  = %d\n",ret, _count++);
			if (ret < 0)
			{
				CELLLog::Info("CELLServer%d,OnRun,selectr���������ERROR\n",m_id);
				pThread->Exit();
				break;
			}
			else if(ret == 0)
			{
				continue;
			}

			ReadData(fd_read);
			WriteData(fd_write);
			//WriteData(fd_Exc);
			
			//CELLLog::Info("CELLServer%d,fd_write=%d.fd_read=%d\n", m_id,fd_write.fd_count,fd_read.fd_count);
			/*if (fd_Exc.fd_count > 0)
			{
				CELLLog::Info("######fd_Exc=%d\n", fd_Exc.fd_count);
			}*/
			//return true;
		}
		CELLLog::Info("CellServer%d,OnRun exit\n",m_id);
		//return false;
	/*	return true;*/
	}

	
	void checkTime()
	{
		auto nowTime = CELLTime::getNowInMilliSec();
		auto dt = nowTime - oldTime;
		oldTime = nowTime;
		for (auto iter = m_vectClients.begin();iter != m_vectClients.end();)
		{
			if (iter->second->checkHeart(dt))
			{
				if (m_pInetEvent)
				{
					m_pInetEvent->OnNetLeave(iter->second);
					auto iterdel = iter++;
					m_client_change = true;
					m_vectClients.erase(iterdel);
					continue;
				}
			}
			/*iter->second->checkSendTime(dt);*/
			iter++;
		}
	}

	void OnClientLeave(std::shared_ptr<CellClient> pClient)
	{
		if (m_pInetEvent)
		{
			m_pInetEvent->OnNetLeave(pClient);
		}
		m_client_change = true;
	}
	void WriteData(fd_set &fd_write)
	{
#ifdef _WIN32
		for (int i = 0; i < fd_write.fd_count; i++)
		{
			SOCKET fd = fd_write.fd_array[i];
			auto iter = m_vectClients.find(fd);
			if (iter != m_vectClients.end())
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					iter = m_vectClients.erase(iter);	
				}
			}
		}
#else
		for (auto iter : m_vectClients)
		{
			if (FD_ISSET(iter.first, &fd_write))
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					iter = m_vectClients.erase(iter);
				}
			}
		}
#endif // _WIN32
	}
	void ReadData(fd_set &fd_read)
	{
#ifdef _WIN32
		for (int i = 0; i < fd_read.fd_count; i++)
		{
			SOCKET fd = fd_read.fd_array[i];
			auto iter = m_vectClients.find(fd);
			if (iter != m_vectClients.end())
			{
				if (-1 == RecvData(iter->second))
				{
					OnClientLeave(iter->second);
					iter = m_vectClients.erase(iter);
				}
			}
		}
#else
		for (auto iter : m_vectClients)
		{
			if (FD_ISSET(iter.first, &fd_read))
			{
				if (-1 == RecvData(iter.second))
				{
					OnClientLeave(iter->second);
					iter = m_vectClients.erase(iter);
				}
			}
		}
#endif // _WIN32
	}

	int RecvData(std::shared_ptr<CellClient>& pClient)
	{

		//���տͻ��˵���������
		int nLen = pClient->RecvData();
		
		if (nLen < 0)
		{
			//CELLLog::Info("�ͻ���<socket = %d>���˳��������������\n", pClient->getSocket());
			return -1;
		}
		//�������������¼�
		m_pInetEvent->OnNetRecv(pClient);

		//ѭ�� �ж��Ƿ�����Ϣ��Ҫ����
		while (pClient->hasMsg())
		{
			//����������Ϣ
			OnNetMsg(pClient, pClient->front_msg());
			//�Ƴ���Ϣ���У�����������ǰ�˵�����
			pClient->pop_front_msg();
		}

		return 0;
	}
	void OnNetMsg(std::shared_ptr<CellClient> pClient, DataHeader* header)
	{
		//����ͻ�������
		m_pInetEvent->OnNetMsg(this, pClient, header);
		/*auto t1 = m_tTime.getElaspedSecond();
		if (t1 >= 1.0)
		{
		CELLLog::Info("time<%lf>,socket<%d>,clientNum<%d>,recvCount<%d>\n", t1, _cSock, m_vectClients.size(), m_recvCount);
		m_tTime.update();
		m_recvCount = 0;
		}*/
	}


	//�ر�
	void Close()
	{
		CELLLog::Info("Cellserver%d Close begin\n", m_id);
		
		//�������
		m_CellTaskServer.Close();
		m_thread.Close();
		CELLLog::Info("Cellserver%d Close end\n",m_id);
	}

	void ClearClients()
	{
		m_vectClients.clear();
		m_vectClientsBuff.clear();
	}
	void addClient(std::shared_ptr<CellClient> pClient)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		//m_mutex.lock();
		m_vectClientsBuff.push_back(pClient);
		//	m_mutex.unlock;
	}

	void Start()
	{
		m_thread.Start(nullptr,
			[this](CELLThread *pThread) {
			OnRun(pThread);},
			[this](CELLThread *pThread) {
				ClearClients(); });
		m_CellTaskServer.Start();
	}

	size_t getClientNum()
	{
		return m_vectClients.size() + m_vectClientsBuff.size();
	}

	void addSendTask(std::shared_ptr<CellClient> pClient, std::shared_ptr<DataHeader> ret)
	{
		//����lambda���ʽʵ�֣����������࣬�߼�̫����
		m_CellTaskServer.addTask([pClient, ret]() 
		{
			if (SOCKET_ERROR == pClient->SendData(ret))
			{

			}
		}
		);
	}
		
private:
	SOCKET m_sock;
	//�ͻ�����ʽ����
	std::map<SOCKET, std::shared_ptr<CellClient> > m_vectClients;
	std::vector<std::shared_ptr<CellClient> > m_vectClientsBuff;//�ͻ��˻������
	std::mutex m_mutex;//���ӿͻ�����,���������
					   //std::thread *m_pthread;
	std::shared_ptr<std::thread> m_pthread;
	//�����¼�����
	INetEvent* m_pInetEvent;
	//��������
	char recvBUF[RECV_BUFF_SIZE] = {};

	CellTaskServer m_CellTaskServer;

	//���ݿͻ���fd_set
	fd_set m_fd_read_back;
	//�ͻ����б��Ƿ�仯
	bool m_client_change;
	//���ͻ���sockֵ
	SOCKET m_maxSock;

	time_t oldTime = CELLTime::getNowInMilliSec();
	
	//�߳�
	CELLThread m_thread;
	//CellServerID
	int m_id = -1;
};



#endif // !_CELLSERVER_HPP_
