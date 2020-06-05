#ifndef _CELLCLIENT_HPP_
#define _CELLCLIENT_HPP_

#include "Cell.hpp"
//�ͻ������ݶ���
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

	//����SOCKET����
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
private:
	SOCKET m_sockfd;
	//������
	char m_szMSGBuf[RECV_BUFF_SIZE];
	//��Ϣ������β��λ��
	int  m_lastPos = 0;

	//���ͻ�����
	char m_szSendMSGBuf[SEND_BUFF_SIZE];
	//��Ϣ������β��λ��
	int  m_SendlastPos = 0;
};
#endif // !_CELLCLIENT_HPP_

