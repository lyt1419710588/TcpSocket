#ifndef _CELLCLIENT_HPP_
#define _CELLCLIENT_HPP_

#include "Cell.hpp"
#include "CELLBuffer.hpp"
//客户端死亡计时时间
#define CLIENT_HEART_DEAD_TIME 60000
//指定时间清空发送缓冲区数据
#define CLIENT_SEND_BUFF_TIME 200
//客户端数据对象
class CellClient :public ObjectPoolBase<CellClient, 1000>
{
public:
	int id = -1;
	int serverID = -1;
public:
	CellClient(SOCKET sock = INVALID_SOCKET):
		m_SendBuff(SEND_BUFF_SIZE),
		m_RecvBuff(RECV_BUFF_SIZE)
	{
		static int n = 1;
		id = n++;
		m_sockfd = sock;
		
		resetDTHeart();
		resetDTSend();
	}

	~CellClient()
	{
		CELLLog_Debug("~CellClient%d,server:%d",id, serverID);
		if (INVALID_SOCKET != m_sockfd)
		{
#ifdef _WIN32
			closesocket(m_sockfd);
#else
			close(m_sockfd);
#endif // _WIN32
			m_sockfd = INVALID_SOCKET;
		}
	}

	SOCKET getSocket()
	{
		return m_sockfd;
	}
	
	int RecvData()
	{
		return m_RecvBuff.readToSocket(m_sockfd);
	}

	bool hasMsg()
	{
		return m_RecvBuff.hasMsg();
	}


	DataHeader* front_msg()
	{
		return (DataHeader*)m_RecvBuff.data();
	}

	void pop_front_msg()
	{
		if (hasMsg())
		{
			m_RecvBuff.pop(front_msg()->dataLength);
		}	
	}
	//理解将缓冲区数据发送给客户端
	int SendDataReal()
	{
		//重置发送时间
		resetDTSend();
		return m_SendBuff.writeToSocket(m_sockfd);
	}
	//发送SOCKET数据
	int SendData(std::shared_ptr<DataHeader> header)
	{
		return SendData((const char*)header.get(), header->dataLength);
	}

	int SendData(const char* pData,int nLen)
	{
		if (m_SendBuff.push(pData, nLen))
		{
			return nLen;
		}
		return SOCKET_ERROR;
	}
	//计时重置
	void resetDTHeart()
	{
		m_dtHeart = 0;
	}
	//重置发送时间
	void resetDTSend()
	{
		m_dtSend = 0;
	}
	//心跳检测
	bool checkHeart(time_t dt)
	{
		m_dtHeart += dt;
		if (m_dtHeart >= CLIENT_HEART_DEAD_TIME)
		{	
			CELLLog_Info("checkheart dead:%d,dt_heart=%d",m_sockfd,m_dtHeart);
			/*m_dtHeart = 0;*/
			return true;
		}
		return false;
	}

	//检查发送时间
	bool checkSendTime(time_t dt)
	{
		m_dtSend += dt;
		if (m_dtSend >= CLIENT_SEND_BUFF_TIME)
		{
			/*CELLLog_Info("senTime now---:%d,dt_sentTime=%d,dt=%d", m_sockfd, m_dtSend,dt);*/
			//立即发送缓存数据
			//重置发送计时
			SendDataReal();
			resetDTSend();
			/*m_dtSend = 0;*/
			return true;
		}
		return false;
	}
	bool needWrite()
	{
		return m_SendBuff.needWrite();
	}
private:
	SOCKET m_sockfd;
	//接收消息
	CELLBuffer m_RecvBuff;
	

	//发送缓冲区
	CELLBuffer m_SendBuff;

	//死亡计时
	time_t m_dtHeart;

	//上次发送消息数据时间
	time_t m_dtSend;

	//发送缓冲区写满的计数
	int m_sendBuffFullCount = 0;
};
#endif // !_CELLCLIENT_HPP_

