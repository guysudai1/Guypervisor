#include "virtualization_detection.h"
#include "virtual_addr_helpers.h"
#include "vmcs.h"
#include "print.h"

#include <intrin.h>
#include <ntifs.h>
#include <wdf.h>

#define ALIGNMENT_SIZE 4*1024 // 4KB

#define IA32_FEATURE_CONTROL_MSR 0x3A
#define IA32_VMX_BASIC 0x480

#define IA32_VMX_CR0_FIXED0 0x486
#define IA32_VMX_CR0_FIXED1 0x487

#define IA32_VMX_CR4_FIXED0 0x488
#define IA32_VMX_CR4_FIXED1 0x489

namespace virtualization {
	bool enter_vmxon_mode()
	{
		// Set CR4.VMXE [bit 13] = 1
		// readmsr 0x3A and modify with:
		//  - Lock bit [0 bit] = 0 (
		//  - SMX Operation [1 bit] = 0
		//  - SMX out of operation [2 bit] = 1
		unsigned long long cr0, cr4, controlMsr;
		PVOID allocatedVMX;
		PHYSICAL_ADDRESS maxPhysical = { 0 };
		unsigned char operationStatus;
		
		// Set CR0 Fixed values
		cr0 = __readcr0();
		cr0 &= __readmsr(IA32_VMX_CR0_FIXED0);
		cr0 |= __readmsr(IA32_VMX_CR0_FIXED1);
		__writecr0(cr0);

		// Set CR4 Fixed values
		cr4 = __readcr4();
		cr4 &= __readmsr(IA32_VMX_CR4_FIXED0);
		cr4 |= __readmsr(IA32_VMX_CR4_FIXED1);
		__writecr4(cr4);

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
			return false;
		}
		RtlZeroMemory(allocatedVMX, sizeof(VMCS) + ALIGNMENT_SIZE);

		// Align to 4KB
		allocatedVMX = reinterpret_cast<PVOID>((reinterpret_cast<uintptr_t>(allocatedVMX) + ALIGNMENT_SIZE - 1) & ~(ALIGNMENT_SIZE - 1));

		operationStatus = __vmx_on(static_cast<UINT64*>(allocatedVMX));
		if (operationStatus) {
			// Failed
			MDbgPrint("Instruction VMXON failed!");
			return false;
		}

		// Just for testing
		__vmx_off();
	}
}