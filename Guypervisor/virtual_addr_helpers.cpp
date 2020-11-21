#include "virtual_addr_helpers.h"

#include <intrin.h>

#include "processor_context.h"
#include "cpuid.h"
#include "print.h"

void AcquireMaxPhysicalAddress(PHYSICAL_ADDRESS& maxPhys)
{
	/*
	2. Software can determine a processor’s physical - address width by executing CPUID with 80000008H in EAX.The physical - address
		width is returned in bits 7:0 of EAX.
	*/
	PHYSICAL_ADDRESS MAX_PHYS = { 0 };
	processor::CPUInfo currentCpu;
	unsigned char phys_width;

	__cpuid(reinterpret_cast<int*>(&currentCpu), cpuid::kIa32AddressWidth);

	// Acquire bits 7:0
	phys_width = currentCpu.regs.eax.all & 0xff; 
	MAX_PHYS.QuadPart = static_cast<LONGLONG>(1) << static_cast<LONGLONG>(phys_width);

	maxPhys = MAX_PHYS;
}

PVOID AllocateContingiousPhysicalMemoryAligned(SIZE_T size, SIZE_T alignmentSize)
{
	PHYSICAL_ADDRESS maxPhysical = { 0 };
	PVOID allocatedMemory;

	// Acquire max physical address
	AcquireMaxPhysicalAddress(maxPhysical);

	// Allocate 4KB aligned memory (not aligned yet)
	allocatedMemory = MmAllocateContiguousMemory(size + alignmentSize, maxPhysical);

	if (!allocatedMemory) {
		MDbgPrint("MmAllocateContiguousMemory cannot allocate %ud bytes with alignment of %ud \n",
				  size, alignmentSize);
		return false;
	}
	RtlZeroMemory(allocatedMemory, size + alignmentSize);

	// Align to 4KB
	allocatedMemory = reinterpret_cast<PVOID>(
		(reinterpret_cast<uintptr_t>(allocatedMemory) + alignmentSize - 1) & ~(alignmentSize - 1)
		);

	return allocatedMemory;
}
