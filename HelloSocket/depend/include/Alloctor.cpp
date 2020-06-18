#include "Alloctor.h"
#include "MemoryMgr.hpp"
void* operator new(size_t length)
{
	return MemoryMgr::Instance().allocMem(length);
}

void operator delete(void* p)
{
	MemoryMgr::Instance().freeMemory(p);
}

void* operator new[](size_t length)
{
	return MemoryMgr::Instance().allocMem(length);
}

void operator delete[](void* p)
{
	MemoryMgr::Instance().freeMemory(p);
}

void* mem_alloc(size_t length)
{
	return malloc(length);
}
void mem_free(void* p)
{
	free(p);
}