#include "cleanup.h"

#include <wdm.h>

#include "print.h"

constexpr ULONG kGuypervisorTag = 'GuyP';

void* operator new(size_t count)
{
    // Allocate new device object in the nonpaged-pool
    void* ret_addr = ExAllocatePoolWithTag(NonPagedPoolExecute, count, kGuypervisorTag);
    if (ret_addr == nullptr) {
        MDbgPrint("Couldn't allocate %ld bytes\n", count);
        // TODO: Handle bad memory allocation
        goto cleanup;
    }

cleanup:
    return ret_addr;
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