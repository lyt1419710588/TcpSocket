#ifndef _MEMORYMGR_HPP
#define _MEMORYMGR_HPP
#include <stdlib.h>
#include <assert.h>
#include <mutex>
#define MAX_MEMORY_SIZE 128

#ifdef _DEBUG
	#include<stdio.h>
	#define xPrintf(...) printf(__VA_ARGS__)
#else
	#define xPrintf(...)
#endif // !_DEBUG

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
	virtual ~MemoryAlloc()
	{
		if (m_pBuf)
		{
			free(m_pBuf);
		}
	}
	///申请内存
	void* allocMem(size_t nLen)
	{
		std::lock_guard<std::mutex> t(m_mutex);
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
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			
		}
		else
		{
			pReturn = m_pHeader;
			assert(0 == pReturn->nRef );
			pReturn->nRef = 1;
			m_pHeader = pReturn->pNext;
		}
		xPrintf("allocMem:%llx,id=%d,size=%d\n", pReturn, pReturn->nID, nLen);
		return ((char*)pReturn + sizeof(MemoryBlock));
	}
	//释放内存
	void freeMemory(void* pMem)
	{
		
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		xPrintf("freeMemory:%llx,id=%d\n", pBlock, pBlock->nID);
		assert(1 == pBlock->nRef);
		
		if (pBlock->bPool)
		{
			std::lock_guard<std::mutex> t(m_mutex);
			if (--pBlock->nRef != 0)
			{
				xPrintf("freeMemory:Error", pBlock, pBlock->nID);
				return;
			}
			pBlock->pNext = m_pHeader;
			m_pHeader = pBlock;
		}
		else
		{
			if (--pBlock->nRef != 0)
			{
				return;
			}
			free(pBlock);
		}
	}
	//初始化内存池
	void InitMemory()
	{
		//断言
		assert(nullptr == m_pBuf);
		if (m_pBuf)
		{
			return;
		}
		xPrintf("内存池：%llx初始化,m_nSize = %d,m_nBlockSize = %d\n", m_pBuf, m_nSize, m_nBlockSize);
		//计算内存池大小
		size_t bufsize = (m_nSize + sizeof(MemoryBlock)) * m_nBlockSize;
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
			MemoryBlock *pTemp = (MemoryBlock *)(m_pBuf + (i * (m_nSize + sizeof(MemoryBlock))));
			pTemp->bPool = true;
			pTemp->nID = i;
			pTemp->nRef = 0;
			pTemp->pAlloc = this;
			pTemp->pNext = nullptr;

			pTempPre->pNext = pTemp;
			pTempPre = pTemp;
		}
	}
protected:
	//内存池首地址
	char* m_pBuf;
	//头部内存单元
	MemoryBlock* m_pHeader;
	//内存池大小
	size_t m_nSize;
	//内存池块数
	size_t m_nBlockSize;
	//锁
	std::mutex m_mutex;
};

template <size_t nSize,size_t nBlockSize>
class MemoryAlloctor :public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		const size_t n = sizeof(void*);

		m_nSize = (nSize / n) * n + (nSize % n ? n : 0);
		m_nBlockSize = nBlockSize;
	}
};
//内存管理工具
class MemoryMgr
{
private:
	MemoryMgr()
	{
		Init_szAlloc(0, 64, &m_mem64);
		Init_szAlloc(65, 128, &m_mem128);
		/*Init_szAlloc(0, 64, &m_mem64);
		Init_szAlloc(65, 128, &m_mem128);
		Init_szAlloc(129, 256, &m_mem256);
		Init_szAlloc(257, 512, &m_mem512);
		Init_szAlloc(513, 1024, &m_mem1024);*/
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
		if (nLen <= MAX_MEMORY_SIZE)
		{
			return m_szAlloc[nLen]->allocMem(nLen);

		}
		else
		{
			MemoryBlock * pReturn = (MemoryBlock *)malloc(nLen + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			//xPrintf("allocMem:%llx,id=%d,size=%d\n",pReturn,pReturn->nID, nLen);
			return ((char*)pReturn + sizeof(MemoryBlock));
		}	
	}
	//释放内存
	void freeMemory(void* pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		if (pBlock->bPool)
		{
			pBlock->pAlloc->freeMemory(pMem);
		}
		else
		{
			if (--pBlock->nRef == 0)
			{
				free(pBlock);
			}	
		}
		
	}

	//增加内存块的引用
	void addRef(void *pMem)
	{
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		pBlock->nRef++;
	}
private:
	//初始化内存池映射数组
	void Init_szAlloc(int nBegin,int nEnd, MemoryAlloc* mem)
	{
		xPrintf("Init_szAlloc(nBegin = %d,nEnd = %d,mem = %llx \n)", nBegin, nEnd, mem);
		for (int n  = nBegin;n <= nEnd; n++)
		{
			m_szAlloc[n] = mem;
		}
	}
private:
	MemoryAlloctor<64, 4000000> m_mem64;
	MemoryAlloctor<128, 1000000> m_mem128;
	//MemoryAlloctor<256, 100000> m_mem256;
	//MemoryAlloctor<512, 100000> m_mem512;
	//MemoryAlloctor<1024, 100000> m_mem1024;
	MemoryAlloc* m_szAlloc[MAX_MEMORY_SIZE + 1];
};


#endif // !_MEMORYMGR_H

