#ifndef _CELLCLIENT_HPP_
#define _CELLCLIENT_HPP_

#include "Cell.hpp"

//�ͻ���������ʱʱ��
#define CLIENT_HEART_DEAD_TIME 60000
//ָ��ʱ����շ��ͻ���������
#define CLIENT_SEND_BUFF_TIME 200
//�ͻ������ݶ���
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

	//��⽫���������ݷ��͸��ͻ���
	int SendDataReal()
	{
		int ret = 0;
		if (m_SendlastPos > 0 && INVALID_SOCKET != m_sockfd)
		{
			// //��������
			ret = send(m_sockfd, m_szSendMSGBuf, m_SendlastPos,0);
			////���������
			m_SendlastPos = 0;
			m_sendBuffFullCount = 0;
			//���÷���ʱ��
			resetDTSend();
		}
		return ret;
	}
	//����SOCKET����
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

	//��ʱ����
	void resetDTHeart()
	{
		m_dtHeart = 0;
	}
	//���÷���ʱ��
	void resetDTSend()
	{
		m_dtSend = 0;
	}
	//�������
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

	//��鷢��ʱ��
	bool checkSendTime(time_t dt)
	{
		m_dtSend += dt;
		if (m_dtSend >= CLIENT_SEND_BUFF_TIME)
		{
			/*printf("senTime now---:%d,dt_sentTime=%d,dt=%d\n", m_sockfd, m_dtSend,dt);*/
			//�������ͻ�������
			//���÷��ͼ�ʱ
			SendDataReal();
			resetDTSend();
			/*m_dtSend = 0;*/
			return true;
		}
		return false;
	}
private:
	SOCKET m_sockfd;
	//������
	char m_szMSGBuf[RECV_BUFF_SIZE * 2];
	//��Ϣ������β��λ��
	int  m_lastPos = 0;

	//���ͻ�����
	char m_szSendMSGBuf[SEND_BUFF_SIZE];
	//��Ϣ������β��λ��
	int  m_SendlastPos = 0;

	//������ʱ
	time_t m_dtHeart;

	//�ϴη�����Ϣ����ʱ��
	time_t m_dtSend;

	//���ͻ�����д���ļ���
	int m_sendBuffFullCount = 0;
};
#endif // !_CELLCLIENT_HPP_

