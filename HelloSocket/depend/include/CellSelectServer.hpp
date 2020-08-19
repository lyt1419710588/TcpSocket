#ifndef _CELLSELECTSERVER_HPP_
#define _CELLSELECTSERVER_HPP_


#include "CELLFDSet.hpp"
#include "CellServer.hpp"
//网络处理收服务
class CellSelectServer:public CellServer
{
public:
	~CellSelectServer()
	{
		Close();
	}
	bool DoNetEvent()
	{
		//fd_set fd_Exc;
		fd_read.zero();
		if (m_client_change)
		{
			m_maxSock = m_vectClients.begin()->second->getSocket();
			for (auto iter : m_vectClients)
			{
				fd_read.add(iter.second->getSocket());
				if (m_maxSock  < iter.second->getSocket())
				{
					m_maxSock = iter.second->getSocket();
				}
			}
			m_fd_read_back.copy(fd_read);
			m_client_change = false;
		}
		else
		{
			fd_read.copy(m_fd_read_back);
		}


		fd_write.zero();
		for (auto iter : m_vectClients)
		{
			//检测需要写数据的客户端
			if (iter.second->needWrite())
			{
				bNeedWrite = true;
				fd_write.add(iter.second->getSocket());
			}
		}
		//memcpy(&fd_write, &m_fd_read_back, sizeof(m_fd_read_back));
		//memcpy(&fd_Exc, &m_fd_read_back, sizeof(m_fd_read_back));

		//nfds是一个整数值，是指fd_set集合中所有描述符(socket)的范围
		//即是所有描述符最大值+1，在windows中这个参数可以写0
		timeval t{ 0,1 };
		int ret = 0;
		if (bNeedWrite)
		{
			ret = select(m_maxSock + 1, fd_read.fdset(), fd_write.fdset(), nullptr, &t);
			bNeedWrite = false;
		}
		else
		{
			ret = select(m_maxSock + 1, fd_read.fdset(), nullptr, nullptr, &t);
		}

		//CELLLog_Info("select ret = %d,count  = %d",ret, _count++);
		if (ret < 0)
		{
			CELLLog_Info("CELLServer%d,OnRun,select任务结束，error<%d>,errormsg<%s>"
				, m_id, errno, strerror(errno));
			return false;
		}
		else if (ret == 0)
		{
			return true;
		}

		ReadData();
		WriteData();
		//WriteData(fd_Exc);

		//CELLLog_Info("CELLServer%d,fd_write=%d.fd_read=%d", m_id,fd_write.fd_count,fd_read.fd_count);
		/*if (fd_Exc.fd_count > 0)
		{
		CELLLog_Info("######fd_Exc=%d", fd_Exc.fd_count);
		}*/
		return true;
	}

	void WriteData()
	{
#ifdef _WIN32

		for (int i = 0; i < fd_write.fdset()->fd_count; i++)
		{
			SOCKET fd = fd_write.fdset()->fd_array[i];
			auto iter = m_vectClients.find(fd);
			if (iter != m_vectClients.end())
			{
				if (SOCKET_ERROR == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					iter = m_vectClients.erase(iter);
				}
			}
		}
#else
		for (auto iter = m_vectClients.begin(); iter != m_vectClients.end(); iter++)
		{
			if (iter->second->needWrite() && fd_write.has(iter->first))
			{
				if (SOCKET_ERROR == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					m_vectClients.erase(iter++);
				}
			}
		}
#endif // _WIN32
	}
	void ReadData()
	{
#ifdef _WIN32
		for (int i = 0; i < fd_read.fdset()->fd_count; i++)
		{
			SOCKET fd = fd_read.fdset()->fd_array[i];
			auto iter = m_vectClients.find(fd);
			if (iter != m_vectClients.end())
			{
				if (SOCKET_ERROR == RecvData(iter->second))
				{
					OnClientLeave(iter->second);
					iter = m_vectClients.erase(iter);
				}
			}
		}
#else
		for (auto iter = m_vectClients.begin(); iter != m_vectClients.end(); iter++)
		{
			if (fd_read.has(iter->first))
			{
				if (SOCKET_ERROR == RecvData(iter->second))
				{
					OnClientLeave(iter->second);
					m_vectClients.erase(iter++);
				}
			}
		}
#endif // _WIN32
	}

private:
	//备份客户端fd_set
	CELLFDSet m_fd_read_back;
	CELLFDSet fd_read;
	CELLFDSet fd_write;
	//最大客户端sock值
	SOCKET m_maxSock;
};



#endif // !_CELLSELECTSERVER_HPP_
