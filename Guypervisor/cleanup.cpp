#include "cleanup.h"

#include <wdm.h>

void* operator new(size_t count)
{
    // Allocate new device object
    return ExAllocatePoolWithTag(NonPagedPoolExecute,
        count,
        'guyp');
}

void operator delete (void* p) {
    // Free device object
    ExFreePoolWithTag(p,
        'guyp');
}

void operator delete (void* p, unsigned __int64 size) {
    UNREFERENCED_PARAMETER(size);
    // Free device object
    ExFreePoolWithTag(p,
        'guyp');
}