#ifndef __CLEANUP_H
#define __CLEANUP_H

void* operator new (size_t count);
void operator delete(void* ptr);
void operator delete(void* ptr, unsigned __int64 size);

#endif /* __CLEANUP_H */