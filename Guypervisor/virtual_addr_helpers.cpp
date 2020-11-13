#include "virtual_addr_helpers.h"
#include "processor_context.h"
#include <intrin.h>

void acquire_max_phys_addr(PHYSICAL_ADDRESS& maxPhys)
{
	/*
	2. Software can determine a processor’s physical - address width by executing CPUID with 80000008H in EAX.The physical - address
		width is returned in bits 7:0 of EAX.
	*/
	PHYSICAL_ADDRESS MAX_PHYS = { 0 };

	Processor::CPUInfo currentCpu;
	__cpuid(reinterpret_cast<int*>(&currentCpu), 0x80000008);

	// Acquire bits 7:0
	unsigned char phys_width = currentCpu.regs.eax & 0xff; 
	MAX_PHYS.QuadPart = static_cast<LONGLONG>(1) << static_cast<LONGLONG>(phys_width);

	maxPhys = MAX_PHYS;
}
