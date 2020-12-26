#ifndef __VIRTUAL_ADDR_HELPERS_H
#define __VIRTUAL_ADDR_HELPERS_H

#include <wdm.h>

#include "custom_status_codes.h"

LONGLONG AcquireMaxPhysicalAddress();
PVOID AllocateContingiousPhysicalMemoryAligned(SIZE_T size, SIZE_T alignmentSize);

BOOLEAN WritePhysicalMemory(HANDLE PhysicalMemory, PVOID Address, SIZE_T Length);
BOOLEAN ReadPhysicalMemory(HANDLE PhysicalMemory, PVOID Address, SIZE_T Length);
HANDLE OpenPhysicalMemory();

#endif /* __VIRTUAL_ADDR_HELPERS_H */