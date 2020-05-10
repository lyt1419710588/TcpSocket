#ifndef _EASYTCPCLIENT_HPP
#define _EASYTCPCLIENT_HPP

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
	#include <Windows.h>
	#include <WinSock2.h>
#else
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <string.h>
	#define SOCKET  int
	#define INVALID_SOCKET (SOCKET)(0)
	#define SOCKET_ERROR (-1)
#endif

#include <stdio.h>
#include "MesssageHeader.hpp"

class EasyTcpClient
{
private:
	SOCKET m_sock;
public:
	EasyTcpClient()
	{
		m_sock = INVALID_SOCKET;
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
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(ver, &data);
#endif
		if (INVALID_SOCKET != m_sock)
		{
			printf("�ر�֮ǰ���ӣ�socket = %d\n",m_sock);
			Close();
		}
		m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == m_sock)
		{
			printf("socket = %d����ʧ��\n",m_sock);
		}
		else
		{
			//printf("socket = %d�����ɹ�\n",m_sock);
		}
		return 0;
	}
	//���ӷ�����
	int Connect(const char* ip,unsigned short port)
	{
		if (INVALID_SOCKET == m_sock)
		{
			//printf("��ʼ��socket\n");
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
		int ret = connect(m_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("socket = %d ���ӷ�����%s,%dʧ��\n",m_sock, ip, port);
		}
		else
		{
			//printf("socket = %d ���ӷ�����%s,%d\n",m_sock, ip, port);
		}
		return ret;
	}

	//�ر�socket
	void Close()
	{
		//�������
		if (m_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(m_sock);
			WSACleanup();
#else
			close(m_sock);
#endif
			m_sock = INVALID_SOCKET;
		}
	}

	//��������

	
	int _count = 0;
	//������������
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fd_read;
			FD_ZERO(&fd_read);
			FD_SET(m_sock, &fd_read);
			timeval tl = { 0,0 };
			int ret = select(m_sock, &fd_read, NULL, NULL, &tl);
			//printf("select ret = %d,count  = %d\n", ret, _count++);
			if (ret < 0)
			{
				printf("select = %d �����˶Ͽ����ӣ��������\n", m_sock);
				Close();
				return false;
			}
			if (FD_ISSET(m_sock, &fd_read))
			{
				FD_CLR(m_sock, &fd_read);
				if (-1 == RecvData())
				{
					printf("select = %d �������2\n",m_sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}
	bool isRun()
	{
		return m_sock != INVALID_SOCKET;
	}
	//��������,����ճ������ְ�
	//�ڶ���������˫����
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240//��������С
#endif // !RECV_BUFF_SIZE


	//���ջ�����
	char m_recvBUF[RECV_BUFF_SIZE] = {};
	//��Ϣ������
	char m_szMsgBUF[RECV_BUFF_SIZE * 2] = {};
	//��Ϣ������β��λ��
	int m_lastPos = 0;
	int RecvData()
	{	
		//���տͻ��˵���������
		int nLen = recv(m_sock, m_recvBUF, RECV_BUFF_SIZE, 0);
		if (nLen < 0)
		{
			printf("��������Ͽ����ӣ������������\n");
			return -1;
		}
		//printf("Recv len = %d\n",nLen);
		//�����յ������ݿ�������Ϣ������
		memcpy(m_szMsgBUF + m_lastPos, m_recvBUF, nLen);
		//��Ϣ������β����λ�ú���
		m_lastPos += nLen;
		//�ж�������Ϣ�����������ݳ����Ƿ������Ϣͷ
		while (m_lastPos >= sizeof(DataHeader))
		{
			DataHeader* header = (DataHeader*)m_szMsgBUF;
			if (m_lastPos >= header->dataLength)
			{
				//ʣ��δ�������Ϣ���������ݵĳ���
				int nSize = m_lastPos - header->dataLength;
				//����������Ϣ
				OnNetMsg(header);
				//��δ���������ǰ��
				memcpy(m_szMsgBUF, m_szMsgBUF + header->dataLength, nSize);
				m_lastPos = nSize;
			}
			else
			{
				break;
			}
		}
		/*DataHeader *header = (DataHeader*)recvBUF;
		if (nLen < 0)
		{
			printf("��������Ͽ����ӣ������������\n");
			return -1;
		}
		//recv(m_sock, recvBUF + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(header);*/
		return 0;
	}
	//��Ӧ��������
	void OnNetMsg(DataHeader* header)
	{
		//����ͻ�������
		switch (header->cmd)
		{
			case CMD_LOGIN_RESULT:
			{
				//���շ��������ص�����
				LoginResult *ret = (LoginResult*)header;
				//printf("�յ��������Ϣ��retLogin = %d�����ݳ���:%d\n", ret->result, header->dataLength);
			}
			break;
			case CMD_LOGOUT_RESULT:
			{
				//���շ��������ص�����
				LogoutResult *ret = (LogoutResult*)header;
				//printf("�յ��������Ϣ��retLogout = %d�����ݳ���:%d\n", ret->result, header->dataLength);
			}
			break;
			case CMD_NEW_USER_JOIN:
			{
				//���շ��������ص�����
				NewUserJoin *ret = (NewUserJoin*)header;
				//printf("�յ��������Ϣ��newUerJoinIN  sock = %d�����ݳ���:%d\n", ret->sock, header->dataLength);
			}
			break;
			case CMD_ERROR:
			{
				//���շ��������ص�����
				printf("�յ��������Ϣ��CMD_ERROR  sock = %d�����ݳ���:%d\n", m_sock, header->dataLength);
			}
			break;
			default:
			{
				printf("�յ������δ�������ݣ�  sock = %d�����ݳ���:%d\n", m_sock, header->dataLength);
			}
			break;
		}
	}
	//��������
	int SendData(DataHeader *header,int nLen)
	{
		if (isRun() && header)
		{
			return send(m_sock, (const char*)header, nLen, 0);
		}
		return SOCKET_ERROR;
	}
private:

};


#endif // !_EASYTCPCLIENT_HPP

