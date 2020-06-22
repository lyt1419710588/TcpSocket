#ifndef _EASYTCPCLIENT_HPP
#define _EASYTCPCLIENT_HPP

#include "Cell.hpp"
#include "CELLNetWork.hpp"
#include "CellClient.hpp"

class EasyTcpClient
{
public:
	EasyTcpClient()
	{
		m_isConnected = false;
	}
	//����������
	virtual ~EasyTcpClient()
	{
		Close();
	}
	//��ʼ��
	int initSocket()
	{
		//���� Win sock 2.x
		CELLNetWork::Init();
		if (_pClient)
		{
			 CELLLog::Info("�ر�֮ǰ���ӣ�socket = %d\n",_pClient->getSocket());
			Close();
		}
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == sock)
		{
			 CELLLog::Info("socket = %d����ʧ��\n", _pClient->getSocket());
		}
		else
		{
			_pClient = new CellClient(sock);
			 //CELLLog::Info("socket = %d�����ɹ�\n", _pClient->getSocket());
		}
		return 0;
	}
	//���ӷ�����
	int Connect(const char* ip,unsigned short port)
	{
		if (!_pClient)
		{
			//CELLLog::Info("��ʼ��socket\n");
			initSocket();
		}
		//����
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		//CELLLog::Info("<socket=%d>�������ӷ�����<%s,%d>\n",_pClient->getSocket(),ip,port);
		int ret = connect(_pClient->getSocket(), (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			 CELLLog::Info("socket = %d ���ӷ�����%s,%dʧ��\n", _pClient->getSocket(), ip, port);
		}
		else
		{
			m_isConnected = true;
		    //CELLLog::Info("socket = %d ���ӷ�����%s,%d\n", _pClient->getSocket(), ip, port);
		}
		return ret;
	}

	//�ر�socket
	void Close()
	{
		//�������
		if (_pClient)
		{
			delete _pClient;
			_pClient = nullptr;
		}
		m_isConnected = false;
	}

	//��������

	
	int _count = 0;
	//������������
	bool OnRun()
	{
		if (isRun())
		{
			SOCKET m_sock = _pClient->getSocket();
			fd_set fd_read;
			
			FD_ZERO(&fd_read);
			FD_SET(m_sock, &fd_read);
			
			fd_set fd_write;
			FD_ZERO(&fd_write);

			timeval tl = { 0,10 };
			int ret;
			if (_pClient->needWrite())
			{
				FD_SET(m_sock, &fd_write);
				ret = select(m_sock, &fd_read, &fd_write, nullptr, &tl);
			}
			else
			{
				 ret = select(m_sock, &fd_read, nullptr, nullptr, &tl);
			}
			
			//  CELLLog::Info("select ret = %d,count  = %d\n", ret, _count++);
			if (ret < 0)
			{
				CELLLog::Info("select = %d �����˶Ͽ����ӣ��������\n", m_sock);
				Close();
				return false;
			}
			if (FD_ISSET(m_sock, &fd_read))
			{
				if (-1 == RecvData())
				{
					CELLLog::Info("select = %d �������2\n",m_sock);
					Close();
					return false;
				}
			}

			if (FD_ISSET(m_sock, &fd_write))
			{
				if (-1 == _pClient->SendDataReal())
				{
					CELLLog::Info("select = %d �������2\n", m_sock);
					Close();
					return false;
				}
			/*	else
				{
					CELLLog::Info("select = %d ���ݷ���\n", m_sock);
				}*/
			}
			return true;
		}
		return false;
	}
	bool isRun()
	{
		return (_pClient) && m_isConnected;
	}
	//��������,����ճ������ְ�
	//�ڶ���������˫����
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240//��������С
#endif // !RECV_BUFF_SIZE


	//���ջ�����
	char m_recvBUF[RECV_BUFF_SIZE] = {};
	//��Ϣ������
	char m_szMsgBUF[RECV_BUFF_SIZE * 4] = {};
	//��Ϣ������β��λ��
	int m_lastPos = 0;
	int RecvData()
	{	
		if (isRun())
		{
			//���տͻ��˵���������
			int nLen = _pClient->RecvData();

			if (nLen > 0)
			{
				while (_pClient->hasMsg())
				{
					//����������Ϣ
					OnNetMsg(_pClient->front_msg());
					//�Ƴ���Ϣ���У�����������ǰ�˵�����
					_pClient->pop_front_msg();
				}
			}
			return nLen;
		}
		return 0;
	}
	//��Ӧ��������
	virtual void OnNetMsg(DataHeader* header)  = 0;
	//��������
	int SendData(std::shared_ptr<DataHeader> data)
	{
		if (isRun())
		{
			return _pClient->SendData(data);
		}
		return 0;
	}
	int SendData(const char* pData, int nLen)
	{
		if (isRun())
		{
			return _pClient->SendData(pData, nLen);
		}
		return 0;
	}
	CellClient *getCurClient()
	{
		return _pClient;
	}
protected:
	CellClient *_pClient = nullptr;
	bool m_isConnected = false;
};


#endif // !_EASYTCPCLIENT_HPP

