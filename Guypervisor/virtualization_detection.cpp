#include "virtualization.h"

#include <intrin.h>

#include "processor_context.h"
#include "cpuid.h"

constexpr const char* kVendorName = "GenuineIntel";

namespace virtualization {
	bool VendorIsIntel()
	{
		/*
		Looks for GenuineIntel string in CPUINFO structure
		*/
		processor::VendorCPUInfo cpuidStruct;
		char vendorString[12];

		// Acquire vendor string
		__cpuid(reinterpret_cast<int*>(&cpuidStruct), cpuid::intel_e::kIa32CpuVendorName);

		memcpy_s(vendorString, 12, cpuidStruct.vendorID.firstPart, 4);
		memcpy_s(vendorString, 12, cpuidStruct.vendorID.secondPart, 4);
		memcpy_s(vendorString, 12, cpuidStruct.vendorID.thirdPart, 4);

		return memcmp(vendorString, kVendorName, 12);
	}

	bool SupportsVtxOperation()
	{
		/*
		Uses CPUID in order to determine whether the current processor
		supports VTX.

		According to intel's manual:
			If CPUID.1:ECX.VMX[bit 5] = 1, then VMX operation is supported
		*/
		processor::CPUInfo currentCpu;

		__cpuid(reinterpret_cast<int*>(&currentCpu), cpuid::intel_e::kIa32CpuVersion);

		// Check if 5th bit (from the right) is on
		return currentCpu.regs.ecx.rev_bitfield.bit5;
	}

}