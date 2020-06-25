#ifndef _ALLOC_H
#define _ALLOC_H

void* operator new(size_t length);

void operator delete(void* p);

void* operator new[](size_t length);

void operator delete[](void* p);

void* mem_alloc(size_t length);
void mem_free(void* p);
#endif // !_ALLOC_H

