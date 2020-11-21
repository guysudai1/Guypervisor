#ifndef __VIRTUAL_ADDR_HELPERS_H
#define __VIRTUAL_ADDR_HELPERS_H

#include <wdm.h>

#define STATUS_FAILED_TO_ALLOCATE_MEMORY 0xC0133713

void AcquireMaxPhysicalAddress(PHYSICAL_ADDRESS& maxPhys);
PVOID AllocateContingiousPhysicalMemoryAligned(SIZE_T size, SIZE_T alignmentSize);

#endif /* __VIRTUAL_ADDR_HELPERS_H */