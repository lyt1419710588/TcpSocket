#ifndef _CELLOBJECTPOOL_H
#define _CELLOBJECTPOOL_H
#include <stdlib.h>
#include <mutex>
#include <assert.h>


#ifdef _DEBUG
	#ifndef xPrintf
	#include<stdio.h>
	#define xPrintf(...) printf(__VA_ARGS__)
	#endif // !xPrintf
#else
	#ifndef xPrintf
	#define xPrintf(...)
	#endif // !xPrintf
#endif // !_DEBUG

template<class T,size_t nSize>
class CellObjectPool
{
public:
	CellObjectPool()
	{
		m_pBuf = nullptr;
		m_pHeader = nullptr;
		InitPool();
	}
	~CellObjectPool()
	{
		if (m_pBuf)
		{
			delete[] m_pBuf;
		}
	}
private:
	struct NodeHeader
	{
		//下一块位置
		NodeHeader *pNext;
		//对象块变化
		int nID;
		//释放在对象池中
		bool bPool;
		//引用次数
		char nRef;
	private:
		char c1;
		char c2;
	};
public:
	//申请对象
	void *allocObjMemory(size_t nSize)
	{
		std::lock_guard<std::mutex> lg(m_mutex);
		NodeHeader* pReturn = nullptr;
		if (nullptr == m_pHeader)
		{
			pReturn = (NodeHeader*)new char[sizeof(T) + sizeof(NodeHeader)];
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->bPool = false;
			pReturn->pNext = nullptr;
		}
		else
		{
			pReturn = m_pHeader;
			m_pHeader = m_pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;
		}
		xPrintf("allocObjMemory:%llx,id=%d,size=%d", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(NodeHeader));
	}
	//释放对象
	void freeObjMemory(void* pMem)
	{

		NodeHeader* pBlock = (NodeHeader*)((char*)pMem - sizeof(NodeHeader));
		xPrintf("freeObjMemory:%llx", pBlock);
		//xPrintf("freeMemory:%llx,id=%d", pBlock, pBlock->nID);
		assert(1 == pBlock->nRef);
		if (pBlock->bPool)
		{
			std::lock_guard<std::mutex> t(m_mutex);
			if (--pBlock->nRef != 0)
			{
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
			delete[] pBlock;
		}
	}
private:
	//初始化对象池
	void InitPool()
	{
		//计算对象池的大小
		size_t n = nSize * (sizeof(T) + sizeof(NodeHeader));
		//像内存池申请内存
		m_pBuf = new char[n];
		//初始化对象池
		m_pHeader = (NodeHeader*)m_pBuf;
		m_pHeader->nID = 0;
		m_pHeader->nRef = 0;
		m_pHeader->bPool = true;
		m_pHeader->pNext = nullptr;

		NodeHeader *pPreTemp = m_pHeader;
		//便利内存块，初始化对象池
		for (size_t i = 1; i < nSize; i++)
		{	
			NodeHeader *pTemp = (NodeHeader*)(m_pBuf + i * (sizeof(T) + sizeof(NodeHeader)));
			pTemp->nID = (int)i;
			pTemp->nRef = 0;
			pTemp->bPool = true;
			pTemp->pNext = nullptr;
			pPreTemp->pNext = pTemp;
			pPreTemp = pTemp;
		}
	}
private:
	//对象池首部
	NodeHeader *m_pHeader;
	//对象池缓存区地址
	char *m_pBuf;
	//锁
	std::mutex m_mutex;
};
template <class T, size_t nSize>
class ObjectPoolBase
{
public:
	void* operator new(size_t length)
	{
		return objectPool().allocObjMemory(length);
	}

	void operator delete(void* p)
	{
		objectPool().freeObjMemory(p);
	}
	//模板可变参数，不定参数
	template<class ...Args>
	static T* createObj(Args... args)
	{
		T *obj = new T(args ...);
		return obj;
	}

	static void deleteObj(T* obj)
	{
		delete obj;
	}
private:
	static CellObjectPool<T,nSize>& objectPool()
	{
		//静态对象池对象
		static CellObjectPool<T, nSize> sPool;
		return  sPool;
	}

};

#endif // !_CELLOBJECTPOOL_H

