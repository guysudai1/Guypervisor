#include "virtualization_detection.h"
#include "virtual_addr_helpers.h"
#include "vmcs.h"
#include "print.h"

#include <intrin.h>
#include <string.h>
#include <ntifs.h>
#include <wdf.h>


#define HAS_CPUID_FLAG_MASK 1 << 21

#define NUMBER_OF_REGISTERS 4 // eax, ebx, ecx, edx
#define SIZE_OF_REGISTER 4 // in bytes

const char* INTEL_VENDOR_STRING = "GenuineIntel";

#define BIT_NUMBER 5
#define UINT32_SIZE 32 // in bits


extern "C" bool is_cpuid_supported(unsigned int mask);

namespace virtualization {

	bool vendor_is_intel()
	{
		/*
		Looks for GenuineIntel string in CPUINFO structure
		*/
		cpuinfo cpuidStruct;
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
		cpuinfo currentCpu;

		__cpuid(reinterpret_cast<int*>(&currentCpu), 1);

		// Check if 5th bit is on
		return currentCpu.regs.ecx & (1 << (UINT32_SIZE - BIT_NUMBER));
	}

}