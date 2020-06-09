#ifndef _CELLSERVER_HPP_
#define _CELLSERVER_HPP_

#include "Cell.hpp"
#include "INetEvent.hpp"
#include "CELLSemaphore.h"

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
	bool OnRun()
	{
		while (m_isRun)
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
			fd_set fd_read;
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

			//nfds��һ������ֵ����ָfd_set����������������(socket)�ķ�Χ
			//�����������������ֵ+1����windows�������������д0
			timeval t{ 0,1 };
			int ret = select(m_maxSock + 1, &fd_read, nullptr, nullptr, &t);
			//printf("select ret = %d,count  = %d\n",ret, _count++);
			/*if (ret < 0)
			{
				printf("select�����˳�\n");
				return false;
			}*/

			ReadData(fd_read);
			checkTime();
			//return true;
		}
		printf("CellServer%d,OnRun exit\n",m_id);
		m_sem.wakeup();
		//return false;
		return true;
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
			iter->second->checkSendTime(dt);
			iter++;
		}
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
					if (m_pInetEvent)
					{
						m_pInetEvent->OnNetLeave(iter->second);
					}
					iter = m_vectClients.erase(iter);
					m_client_change = true;
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
					if (m_pInetEvent)
					{
						m_pInetEvent->OnNetLeave(iter.second);
					}
					iter = m_vectClients.erase(iter);
					m_client_change = true;
				}
			}
		}
#endif // _WIN32
	}

	int RecvData(std::shared_ptr<CellClient>& pClient)
	{

		//���տͻ��˵���������
		int nLen = recv(pClient->getSocket(), recvBUF, RECV_BUFF_SIZE, 0);
		m_pInetEvent->OnNetRecv(pClient);
		//printf("Recv len = %d\n", nLen);
		//DataHeader *header = (DataHeader*)recvBUF;
		if (nLen < 0)
		{
			//printf("�ͻ���<socket = %d>���˳��������������\n", pClient->getSocket());
			return -1;
		}

		memcpy(pClient->msgBuf() + pClient->getLast(), recvBUF, nLen);
		//��Ϣ������β����λ�ú���
		pClient->setLast(pClient->getLast() + nLen);
		//�ж�������Ϣ�����������ݳ����Ƿ������Ϣͷ
		while (pClient->getLast() >= sizeof(DataHeader))
		{
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			if (pClient->getLast() >= header->dataLength)
			{
				//ʣ��δ�������Ϣ���������ݵĳ���
				int nSize = pClient->getLast() - header->dataLength;
				//����������Ϣ
				OnNetMsg(pClient, header);
				//��δ���������ǰ��
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				pClient->setLast(nSize);;
			}
			else
			{
				break;
			}
		}

		//LoginResult ret;
		//SendData(pClient->getSocket(), &ret);
		/* recv(_cSock, recvBUF + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(_cSock, header);*/
		return 0;
	}
	void OnNetMsg(std::shared_ptr<CellClient> pClient, DataHeader* header)
	{
		//����ͻ�������
		m_pInetEvent->OnNetMsg(this, pClient, header);
		/*auto t1 = m_tTime.getElaspedSecond();
		if (t1 >= 1.0)
		{
		printf("time<%lf>,socket<%d>,clientNum<%d>,recvCount<%d>\n", t1, _cSock, m_vectClients.size(), m_recvCount);
		m_tTime.update();
		m_recvCount = 0;
		}*/
	}


	//�ر�
	void Close()
	{
		printf("Cellserver%d Close begin\n", m_id);
		if (m_isRun)
		{
			//�������
			m_CellTaskServer.Close();
			m_isRun = false;
			m_sem.wait();
			m_vectClients.clear();
			m_vectClientsBuff.clear();	
		}
		printf("Cellserver%d Close end\n",m_id);
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
		if (!m_isRun)
		{
			m_isRun = true;
			m_pthread = std::make_shared<std::thread>(std::mem_fn(&CellServer::OnRun), this);
			m_pthread->detach();
			m_CellTaskServer.Start();
		}
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
			pClient->SendData(ret);
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
	//
	CELLSemaphore m_sem;
	//�Ƿ�����
	bool m_isRun = false;
	//CellServerID
	int m_id = -1;
};



#endif // !_CELLSERVER_HPP_
