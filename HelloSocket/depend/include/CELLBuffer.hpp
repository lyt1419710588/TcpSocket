#ifndef _CELL_BUFFER_HPP_
#define _CELL_BUFFER_HPP_

#include "Cell.hpp"

class CELLBuffer
{
public:
	CELLBuffer(int nSize = 8192)
	{
		m_nSize = nSize;
		m_pBuff = new char[m_nSize];
	}
	~CELLBuffer()
	{
		if (m_pBuff)
		{
			delete[] m_pBuff;
			m_pBuff = nullptr;
		}
	}

	char* data()
	{
		return m_pBuff;
	}

	void pop(int nLen)
	{
		int n = m_nLastPos - nLen;
		if (n > 0)
		{
			memcpy(m_pBuff, m_pBuff + nLen, n);
		}
		if (m_nBuffFull > 0)
		{
			m_nBuffFull--;
		}
		m_nLastPos = n;
	}
	bool push(const char* pData, int nLen)
	{
		if (m_nLastPos + nLen <= m_nSize)
		{
			//将要发送的数据拷贝到数据缓冲区
			memcpy(m_pBuff + m_nLastPos, pData, nLen);
			//计算数据尾部位置
			m_nLastPos += nLen;
			if (m_nLastPos == m_nSize)
			{
				m_nBuffFull++;
			}
			return true;
		}
		else
		{
			m_nBuffFull++;
		}
		return false;
	}

	int writeToSocket(SOCKET sockfd)
	{
		int ret = 0;
		if (m_nLastPos > 0 && INVALID_SOCKET != sockfd)
		{
			ret = send(sockfd, m_pBuff, m_nLastPos, 0);
			if (ret <= 0)
			{
				return SOCKET_ERROR;
			}
			if (ret == m_nLastPos)
			{
				m_nLastPos = 0;
			}
			else
			{
				m_nLastPos -= ret;
				memcpy(m_pBuff, m_pBuff + ret,m_nLastPos);
			}
			m_nBuffFull = 0;
		}
		return ret;
	}

	int readToSocket(SOCKET sockfd)
	{
		int ret = 0;
		if (m_nSize - m_nLastPos > 0)
		{
			char* szRecv = m_pBuff + m_nLastPos;
			ret = recv(sockfd, szRecv, m_nSize - m_nLastPos, 0);
			if (ret < 0)
			{
				return SOCKET_ERROR;
			}
			m_nLastPos += ret;
			return ret;
		}
		return 0;
	}

	bool hasMsg()
	{
		if (m_nLastPos >= sizeof(DataHeader))
		{
			DataHeader *header = (DataHeader*)m_pBuff;
			return m_nLastPos >= header->dataLength;
		}
		return false;
	}

	bool needWrite()
	{
		return m_nLastPos > 0;
	}
private:

	//缓冲区
	char* m_pBuff = nullptr;
	//缓冲区数据尾部位置，已有数据长度
	int m_nLastPos = 0;
	//缓冲区大小,字节长度
	int m_nSize = 0;
	//缓冲区放满次数计数
	int m_nBuffFull = 0;
};



#endif // !_CELL_BUFFER_HPP_

