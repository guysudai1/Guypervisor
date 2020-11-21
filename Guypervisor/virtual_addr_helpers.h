#ifndef __VIRTUAL_ADDR_HELPERS_H
#define __VIRTUAL_ADDR_HELPERS_H

#include <wdm.h>

#include "custom_status_codes.h"

void AcquireMaxPhysicalAddress(PHYSICAL_ADDRESS& maxPhys);
PVOID AllocateContingiousPhysicalMemoryAligned(SIZE_T size, SIZE_T alignmentSize);

#endif /* __VIRTUAL_ADDR_HELPERS_H */