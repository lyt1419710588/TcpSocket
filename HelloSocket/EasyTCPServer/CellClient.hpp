#ifndef _CELLCLIENT_HPP_
#define _CELLCLIENT_HPP_

#include "Cell.hpp"
#include "CELLBuffer.hpp"
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
		CELLLog::Info("~CellClient%d,server:%d\n",id, serverID);
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
	//��⽫���������ݷ��͸��ͻ���
	int SendDataReal()
	{
		//���÷���ʱ��
		resetDTSend();
		return m_SendBuff.writeToSocket(m_sockfd);
	}
	//����SOCKET����
	int SendData(std::shared_ptr<DataHeader> header)
	{
		int ret = SOCKET_ERROR;
		if (header)
		{
			
			if (m_SendBuff.push((const char*)header.get(),header->dataLength))
			{
				return header->dataLength;
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
			CELLLog::Info("checkheart dead:%d,dt_heart=%d\n",m_sockfd,m_dtHeart);
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
			/*CELLLog::Info("senTime now---:%d,dt_sentTime=%d,dt=%d\n", m_sockfd, m_dtSend,dt);*/
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
	//������Ϣ
	CELLBuffer m_RecvBuff;
	

	//���ͻ�����
	CELLBuffer m_SendBuff;

	//������ʱ
	time_t m_dtHeart;

	//�ϴη�����Ϣ����ʱ��
	time_t m_dtSend;

	//���ͻ�����д���ļ���
	int m_sendBuffFullCount = 0;
};
#endif // !_CELLCLIENT_HPP_

