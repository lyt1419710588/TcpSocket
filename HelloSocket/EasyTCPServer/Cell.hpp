#ifndef _CELL_HPP_
#define _CELL_HPP_

//SOCKET���
#ifdef _WIN32
#define FD_SETSIZE 10024
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#define SOCKET  int
#define INVALID_SOCKET (SOCKET)(0)
#define SOCKET_ERROR (-1)
#endif

//�Զ���
#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"
#include "CELLTask.hpp"
#include "CELLIObjectPool.hpp"

//������
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240 * 2//��������С
#define SEND_BUFF_SIZE RECV_BUFF_SIZE //���ͻ����� 
#endif // !RECV_BUFF_SIZE

#endif // !_CELL_HPP_

