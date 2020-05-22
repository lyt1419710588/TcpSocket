#ifndef _MEMORYMGR_HPP
#define _MEMORYMGR_HPP
#include <stdlib.h>
#include <assert.h>
class MemoryAlloc;
//内存快
class MemoryBlock
{
public:
	//内存块编号
	int nID;
	//引用次数
	int nRef;
	//所属大内存块
	MemoryAlloc* pAlloc;
	//下一块位置
	MemoryBlock* pNext;
	//是否在内存池中
	bool bPool;
private:
	//预留
	char cNull[3];
};

//内存池
class MemoryAlloc
{
public:
	MemoryAlloc()
	{
		m_pBuf = nullptr;
		m_pHeader = nullptr;
		m_nSize = 0;
		m_nBlockSize = 0;
	}
	~MemoryAlloc()
	{

	}
	///申请内存
	void* allocMem(size_t nLen)
	{
		if (!m_pBuf)
		{
			InitMemory();
		}
		MemoryBlock *pReturn = nullptr;

		if (nullptr == m_pHeader)
		{
			pReturn = (MemoryBlock *)malloc(nLen + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 0;
			pReturn->pAlloc = this;
			pReturn->pNext = nullptr;
		}
		else
		{
			pReturn = m_pHeader;
			assert(0 == pReturn->nRef );
			pReturn->nRef = 1;
			m_pHeader = pReturn->pNext;
		}
		return ((char*)pReturn + sizeof(MemoryBlock));
	}
	//释放内存
	void freeMemory(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		assert(1 == pBlock->nRef);
		if (--pBlock->nRef != 0)
		{
			return;
		}
		if (pBlock->bPool)
		{
			pBlock->pNext = m_pHeader;
			m_pHeader = pBlock;
		}
		else
		{
			free(pMem);
		}
	}
	//初始化内存池
	void InitMemory()
	{
		//断言
		assert(nullptr == m_pBuf);
		if (!m_pBuf)
		{
			return;
		}
		//计算内存池大小
		size_t bufsize = m_nSize * m_nBlockSize;
		//像系统申请池的内存
		m_pBuf = (char*)malloc(bufsize);
		//初始化内存池
		m_pHeader = (MemoryBlock*)m_pBuf;
		m_pHeader->bPool = true;
		m_pHeader->nID = 0;
		m_pHeader->nRef = 0;
		m_pHeader->pAlloc = this;
		m_pHeader->pNext = nullptr;

		
		MemoryBlock *pTempPre = m_pHeader;
		for (size_t i = 1; i < m_nBlockSize; i++)
		{
			MemoryBlock *pTemp = (MemoryBlock *)(m_pBuf + (i * m_nSize));
			pTemp->bPool = true;
			pTemp->nID = 0;
			pTemp->nRef = 0;
			pTemp->pAlloc = this;
			pTemp->pNext = nullptr;

			pTempPre->pNext = pTemp;
			pTempPre = pTemp;
		}
	}
private:
	//内存池首地址
	char* m_pBuf;
	//头部内存单元
	MemoryBlock* m_pHeader;
	//内存池大小
	size_t m_nSize;
	//内存池块数
	size_t m_nBlockSize;
};



//内存管理工具
class MemoryMgr
{
private:
	MemoryMgr()
	{

	}
	~MemoryMgr()
	{

	}
public:
	static MemoryMgr& Instance()
	{
		//单例
		static MemoryMgr mgr;
		return mgr;
	}
	//申请内存
	void* allocMem(size_t nLen)
	{
		return malloc(nLen);
	}
	//释放内存
	void freeMemory(void* p)
	{
		free(p);
	}
private:

};


#endif // !_MEMORYMGR_H

