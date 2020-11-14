#include "virtual_addr_helpers.h"

#include <intrin.h>

#include "processor_context.h"


#define ADDRESS_WIDTH_CPUID 0x80000008


void acquire_max_phys_addr(PHYSICAL_ADDRESS& maxPhys)
{
	/*
	2. Software can determine a processor’s physical - address width by executing CPUID with 80000008H in EAX.The physical - address
		width is returned in bits 7:0 of EAX.
	*/
	PHYSICAL_ADDRESS MAX_PHYS = { 0 };
	processor::CPUInfo currentCpu;
	unsigned char phys_width;

	__cpuid(reinterpret_cast<int*>(&currentCpu), ADDRESS_WIDTH_CPUID);

	// Acquire bits 7:0
	phys_width = currentCpu.regs.eax.all & 0xff; 
	MAX_PHYS.QuadPart = static_cast<LONGLONG>(1) << static_cast<LONGLONG>(phys_width);

	maxPhys = MAX_PHYS;
}
