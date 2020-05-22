#ifndef _MEMORYMGR_HPP
#define _MEMORYMGR_HPP
#include <stdlib.h>
#include <assert.h>
class MemoryAlloc;
//�ڴ��
class MemoryBlock
{
public:
	//�ڴ����
	int nID;
	//���ô���
	int nRef;
	//�������ڴ��
	MemoryAlloc* pAlloc;
	//��һ��λ��
	MemoryBlock* pNext;
	//�Ƿ����ڴ����
	bool bPool;
private:
	//Ԥ��
	char cNull[3];
};

//�ڴ��
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
	///�����ڴ�
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
	//�ͷ��ڴ�
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
	//��ʼ���ڴ��
	void InitMemory()
	{
		//����
		assert(nullptr == m_pBuf);
		if (!m_pBuf)
		{
			return;
		}
		//�����ڴ�ش�С
		size_t bufsize = m_nSize * m_nBlockSize;
		//��ϵͳ����ص��ڴ�
		m_pBuf = (char*)malloc(bufsize);
		//��ʼ���ڴ��
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
	//�ڴ���׵�ַ
	char* m_pBuf;
	//ͷ���ڴ浥Ԫ
	MemoryBlock* m_pHeader;
	//�ڴ�ش�С
	size_t m_nSize;
	//�ڴ�ؿ���
	size_t m_nBlockSize;
};



//�ڴ������
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
		//����
		static MemoryMgr mgr;
		return mgr;
	}
	//�����ڴ�
	void* allocMem(size_t nLen)
	{
		return malloc(nLen);
	}
	//�ͷ��ڴ�
	void freeMemory(void* p)
	{
		free(p);
	}
private:

};


#endif // !_MEMORYMGR_H

