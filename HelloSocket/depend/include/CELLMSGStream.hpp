#ifndef _CELL_MSG_STAREAM_HPP_
#define _CELL_MSG_STAREAM_HPP_
#include "Cell.hpp"
#include "CELLStream.hpp"
class CELLRecvStream:public CELLStream
{
public:
	CELLRecvStream(DataHeader* pHeader):CELLStream((char*)pHeader,pHeader->dataLength)
	{
		push(pHeader->dataLength);
		//��ȡ����
		readInt16();
		//��ȡ����
		getNetCmd();
	}

	uint16_t getNetCmd()
	{
		uint16_t cmd = CMD_ERROR;
		Read<uint16_t>(cmd);
		return cmd;
	}
private:

};

class CELLSendStream :public CELLStream
{
public:
	CELLSendStream(int nSize = 1024) :CELLStream(nSize)
	{
		//Ԥ��ռ�쳤����Ҫ�Ŀռ�
		Write<uint16_t>(0);
	}

	CELLSendStream(char* pData, int nSize = 1024, bool bDel = false) :CELLStream(pData, nSize, bDel)
	{
		//Ԥ��ռ�쳤����Ҫ�Ŀռ�
		Write<uint16_t>(0);
	}

	void setNetCmd(uint16_t cmd)
	{
		Write<uint16_t>(cmd);
	}
	void finish()
	{
		int pos = length();
		setWritePos(0);
		Write<uint16_t>(pos);
		setWritePos(pos);
	}

	bool WriteString(const char* str, int length)
	{
		return writeArray(str, length);
	}
	bool WriteString(const char* str)	
	{
		return writeArray(str, strlen(str));
	}

	bool WriteString(const std::string str)
	{
		return writeArray(str.c_str(), str.length());
	}
private:

};


#endif // !_CELL_MSG_STAREAM_HPP_
