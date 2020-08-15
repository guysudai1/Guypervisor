#include "virtualization_detection.h"
#include "virtual_addr_helpers.h"
#include "vmcs.h"
#include "print.h"

#include <intrin.h>
#include <string.h>
#include <ntifs.h>
#include <wdf.h>

#define ALIGNMENT_SIZE 4*1024 // 4KB

#define HAS_CPUID_FLAG_MASK 1 << 21

#define NUMBER_OF_REGISTERS 4 // eax, ebx, ecx, edx
#define SIZE_OF_REGISTER 4 // in bytes

const char* INTEL_VENDOR_STRING = "GenuineIntel";

#define BIT_NUMBER 5
#define UINT32_SIZE 32 // in bits

#define IA32_FEATURE_CONTROL_MSR 0x3A

extern "C" bool is_cpuid_supported(unsigned int mask);
extern "C" 

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

	void enter_vmxon_mode()
	{
		// Set CR4.VMXE [bit 13] = 1
		// readmsr 0x3A and modify with:
		//  - Lock bit [0 bit] = 0 (
		//  - SMX Operation [1 bit] = 0
		//  - SMX out of operation [2 bit] = 1
		unsigned long long cr4, controlMsr;
		PVOID allocatedVMX;
		PHYSICAL_ADDRESS maxPhysical = { 0 };
		unsigned char operationStatus;

		// Acquire max physical address
		acquire_max_phys_addr(maxPhysical);

		// Set CR4.VMXE = 1
		cr4 = __readcr4();
		cr4 |= (1 << 13);
		__writecr4(cr4);
		
		// readmsr 0x3A and modify lock bit
		controlMsr = __readmsr(IA32_FEATURE_CONTROL_MSR);
		
		// Lock bit = 0
		controlMsr &= 0xfffffffe;

		// SMX Operation = 0
		controlMsr &= 0xffffffff & 0b01;

		// SMX out of operation = 1
		controlMsr |= 0b100;

		__writemsr(IA32_FEATURE_CONTROL_MSR, controlMsr);


		// Allocate 4KB aligned memory (not aligned yet)
		allocatedVMX = MmAllocateContiguousMemory(sizeof(VMCS) + ALIGNMENT_SIZE, maxPhysical);

		if (!allocatedVMX) {
			MDbgPrint("MmAllocateContiguousMemory cannot allocate");
			return;
		}
		RtlZeroMemory(allocatedVMX, sizeof(VMCS) + ALIGNMENT_SIZE);
		// Align to 4KB
		allocatedVMX = reinterpret_cast<PVOID>((reinterpret_cast<uintptr_t>(allocatedVMX) + ALIGNMENT_SIZE - 1) & ~(ALIGNMENT_SIZE - 1));

		operationStatus = __vmx_on(static_cast<unsigned long long*>(allocatedVMX));
		if (operationStatus) {
			// Failed
		}
	}

}