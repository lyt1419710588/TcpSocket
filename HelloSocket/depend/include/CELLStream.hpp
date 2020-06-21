#ifndef _CELL_STAREAM_HPP_
#define _CELL_STAREAM_HPP_
#include <cstdint>

class CELLStream
{
public:
	CELLStream(int nSize = 1024)
	{
		_nSize = nSize;
		_pBuff = new char[_nSize];
	}

	CELLStream(char* pData,int nSize = 1024,bool bDel = false)
	{
		_bDelete = bDel;
		_nSize = nSize;
		_pBuff = pData;
	}
	virtual ~CELLStream()
	{
		if (_pBuff && _bDelete)
		{
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}

	char* Data()
	{
		return _pBuff;
	}
	int length()
	{
		return _nWritePos;
	}
	inline void setWritePos(int n)
	{
		_nWritePos = n;
	}
protected:
	inline bool canRead(uint32_t n)
	{
		return _nSize - _nReadPos >= n;
	}
	inline bool canWrite(uint32_t n)
	{
		return _nSize - _nWritePos >= n;
	}
	inline void push(uint32_t n)
	{
		_nWritePos += n;
	}

	inline void pop(uint32_t n)
	{
		_nReadPos += n;
	}
public:
	///read
	template<typename T>
	bool Read(T& ret,bool bOffset = true)
	{
		auto nLen = sizeof(T);
		if (canRead(nLen))
		{
			memcpy(&ret, _pBuff + _nReadPos, nLen);
			if (bOffset)
			{
				pop(nLen);
			}
			return true;
		}
		return false;
	}
	template<typename T>
	bool onlyRead(T &n)
	{
		return Read(n, false);
	}
	//读取数组
	template<typename T>
	uint32_t ReadArray(T *pData,uint32_t  nLen)
	{
		//读取数组元素个数
		uint32_t tLen = 0;
		//读取数据长度，但不便宜读取位置
		Read(tLen,false);
		if (tLen < nLen)
		{
			auto tSize = tLen * sizeof(T);
			if (canRead(tSize + sizeof(uint32_t)))
			{
				pop(sizeof(uint32_t));
				memcpy(pData, _pBuff + _nReadPos, tSize);
				pop(tSize);
				return tLen;
			}
		}
		return 0;
	}
	////read
	int8_t  readInt8()
	{
		int8_t n = 0;
		Read(n);
		return n;
	}
	int16_t readInt16()
	{
		int16_t n = 0;
		Read(n);
		return n;
	}
	int32_t readInt32()
	{
		int32_t n = 0;
		Read(n);
		return n;
	}
	float   readFloat()
	{
		float n = 0.0f;
		Read(n);
		return n;
	}
	double  readDouble()
	{
		double n = 0.0f;
		Read(n);
		return n;
	}

	


	////write
	bool writeInt8(int8_t n)
	{
		return Write(n);
	}

	bool writeInt16(int16_t n)
	{
		return Write(n);
	}
	
	bool writeInt32(int32_t n)
	{
		return Write(n);
	}

	bool writeFloat(float n)
	{
		return Write(n);
	}
	bool writeDouble(double n)
	{
		return Write(n);
	}

	template<typename T>
	bool writeArray(T *pData, uint32_t nLen)
	{
		auto aLen = sizeof(T) * nLen;
		if (canWrite(aLen + sizeof(uint32_t)))
		{
			writeInt32(nLen);
			memcpy(_pBuff + _nWritePos, pData, aLen);
			push(aLen);
			return true;
		}
		return false;
	}
	
	template<typename T>
	bool Write(T n)
	{
		auto nLen = sizeof(T);
		if (canWrite(nLen))
		{
			memcpy(_pBuff + _nWritePos, &n, nLen);
			push(nLen);
			return true;
		}
		return false;
	}
private:
	//数据缓冲区
	char* _pBuff = nullptr;
	//缓冲区的大小
	int _nSize = 0;

	//已读取的数据尾部位置
	int _nReadPos = 0;
	//已写到的数据尾部位置
	int _nWritePos = 0;
	//缓冲区是否应该被释放
	bool _bDelete = true;

};
#endif // !_CELL_STAREAM_HPP_
