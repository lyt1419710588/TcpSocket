#ifndef _CELLCLIENT_HPP_
#define _CELLCLIENT_HPP_

#include "Cell.hpp"

//客户端死亡计时时间
#define CLIENT_HEART_DEAD_TIME 5000
//客户端数据对象
class CellClient :public ObjectPoolBase<CellClient, 1000>
{
public:
	CellClient(SOCKET sock = INVALID_SOCKET)
	{
		m_sockfd = sock;
		memset(m_szMSGBuf, 0, sizeof(m_szMSGBuf));
		memset(m_szSendMSGBuf, 0, sizeof(m_szSendMSGBuf));
		m_lastPos = 0;
		m_SendlastPos = 0;
		resetDTHeart();
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

	//发送SOCKET数据
	int SendData(std::shared_ptr<DataHeader> header)
	{
		int ret = SOCKET_ERROR;
		if (header)
		{
			const char* pSendData = (const char*)header.get();
			int nSendLen = header->dataLength;
			while (true)
			{
				if (m_SendlastPos + nSendLen >= SEND_BUFF_SIZE)
				{
					int nCopyLen = SEND_BUFF_SIZE - m_SendlastPos;
					memcpy(m_szSendMSGBuf + m_SendlastPos, pSendData, nCopyLen);
					ret = send(m_sockfd, m_szSendMSGBuf, SEND_BUFF_SIZE, 0);
					nSendLen -= nCopyLen;
					pSendData += nCopyLen;
					m_SendlastPos = 0;
					if (SOCKET_ERROR == ret)
					{
						return ret;
					}
				}
				else
				{
					memcpy(m_szSendMSGBuf + m_SendlastPos, pSendData, nSendLen);
					m_SendlastPos += nSendLen;
					break;
				}
			}


		}
		return ret;
	}

	//计时重置
	void resetDTHeart()
	{
		m_dtHeart = 0;
	}
	//检测客户端是否断开
	bool checkHeart(time_t dt)
	{
		m_dtHeart += dt;
		if (m_dtHeart >= CLIENT_HEART_DEAD_TIME)
		{	
			printf("checkheart dead:%d,dt_heart=%d\n",m_sockfd,m_dtHeart);
			m_dtHeart = 0;
			return true;
		}
		return false;
	}
private:
	SOCKET m_sockfd;
	//缓冲区
	char m_szMSGBuf[RECV_BUFF_SIZE];
	//消息缓冲区尾部位置
	int  m_lastPos = 0;

	//发送缓冲区
	char m_szSendMSGBuf[SEND_BUFF_SIZE];
	//消息缓冲区尾部位置
	int  m_SendlastPos = 0;

	//死亡计时
	time_t m_dtHeart;
};
#endif // !_CELLCLIENT_HPP_

