#ifndef _CELLCLIENT_HPP_
#define _CELLCLIENT_HPP_

#include "Cell.hpp"

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
	CellClient(SOCKET sock = INVALID_SOCKET)
	{
		static int n = 1;
		id = n++;
		m_sockfd = sock;
		memset(m_szMSGBuf, 0, sizeof(m_szMSGBuf));
		memset(m_szSendMSGBuf, 0, sizeof(m_szSendMSGBuf));
		m_lastPos = 0;
		m_SendlastPos = 0;
		resetDTHeart();
		resetDTSend();
	}

	~CellClient()
	{
		printf("~CellClient%d,server:%d\n",id, serverID);
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
	char *msgBuf()
	{
		return m_szMSGBuf;
	}

	int getLast()
	{
		return m_lastPos;
	}

	void setLast(int last)
	{
		m_lastPos = last;
	}

	//理解将缓冲区数据发送给客户端
	int SendDataReal()
	{
		int ret = 0;
		if (m_SendlastPos > 0 && INVALID_SOCKET != m_sockfd)
		{
			// //发送数据
			ret = send(m_sockfd, m_szSendMSGBuf, m_SendlastPos,0);
			////情况缓冲区
			m_SendlastPos = 0;
			m_sendBuffFullCount = 0;
			//重置发送时间
			resetDTSend();
		}
		return ret;
	}
	//发送SOCKET数据
	int SendData(std::shared_ptr<DataHeader> header)
	{
		int ret = SOCKET_ERROR;
		if (header)
		{
			const char* pSendData = (const char*)header.get();
			int nSendLen = header->dataLength;
		
			if (m_SendlastPos + nSendLen <= SEND_BUFF_SIZE)
			{
				memcpy(m_szSendMSGBuf + m_SendlastPos, pSendData, nSendLen);
				m_SendlastPos += nSendLen;
				if (m_SendlastPos == SEND_BUFF_SIZE)
				{
					m_sendBuffFullCount++;
				}
				return nSendLen;
			}
			else
			{
				m_sendBuffFullCount++;
			}
		}
		return ret;
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
			printf("checkheart dead:%d,dt_heart=%d\n",m_sockfd,m_dtHeart);
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
			/*printf("senTime now---:%d,dt_sentTime=%d,dt=%d\n", m_sockfd, m_dtSend,dt);*/
			//立即发送缓存数据
			//重置发送计时
			SendDataReal();
			resetDTSend();
			/*m_dtSend = 0;*/
			return true;
		}
		return false;
	}
private:
	SOCKET m_sockfd;
	//缓冲区
	char m_szMSGBuf[RECV_BUFF_SIZE * 2];
	//消息缓冲区尾部位置
	int  m_lastPos = 0;

	//发送缓冲区
	char m_szSendMSGBuf[SEND_BUFF_SIZE];
	//消息缓冲区尾部位置
	int  m_SendlastPos = 0;

	//死亡计时
	time_t m_dtHeart;

	//上次发送消息数据时间
	time_t m_dtSend;

	//发送缓冲区写满的计数
	int m_sendBuffFullCount = 0;
};
#endif // !_CELLCLIENT_HPP_

