#include <wdm.h>
#include <intrin.h>
#include <string.h>

#include "processor_context.h"
#include "virtual_addr_helpers.h"
#include "vmcs.h"
#include "print.h"


const char* INTEL_VENDOR_STRING = "GenuineIntel";
#define BIT_NUMBER 5

extern "C" bool is_cpuid_supported(unsigned int mask);

namespace virtualization {

	bool vendor_is_intel()
	{
		/*
		Looks for GenuineIntel string in CPUINFO structure
		*/
		Processor::CPUInfo cpuidStruct;
		char vendorString[12];

		// Acquire vendor string
		__cpuid(reinterpret_cast<int*>(&cpuidStruct), 0);

		memcpy_s(vendorString, 12, cpuidStruct.vendorID.firstPart, 4);
		memcpy_s(vendorString, 12, cpuidStruct.vendorID.secondPart, 4);
		memcpy_s(vendorString, 12, cpuidStruct.vendorID.thirdPart, 4);

		return memcmp(vendorString, INTEL_VENDOR_STRING, 12);
	}

	bool supports_vtx_operation()
	{
		/*
		Uses CPUID in order to determine whether the current processor
		supports VTX.

		According to intel's manual:
			If CPUID.1:ECX.VMX[bit 5] = 1, then VMX operation is supported
		*/
		Processor::CPUInfo currentCpu;

		__cpuid(reinterpret_cast<int*>(&currentCpu), 1);

		// Check if 5th bit is on
		return currentCpu.regs.ecx & (1 << (sizeof(UINT32) * 8 - BIT_NUMBER));
	}

}