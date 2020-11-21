#include "cleanup.h"

#include <wdm.h>

constexpr ULONG kGuypervisorTag = 'GuyP';

void* operator new(size_t count)
{
    // Allocate new device object in the nonpaged-pool
    return ExAllocatePoolWithTag(NonPagedPoolExecute, count, kGuypervisorTag);
}

void operator delete (void* p) {
    // Free device object
    ExFreePoolWithTag(p, kGuypervisorTag);
}

void operator delete (void* p, unsigned __int64 size) {
    UNREFERENCED_PARAMETER(size);
    // Free device object
    ExFreePoolWithTag(p, kGuypervisorTag);
}