#include "virtualization.h"

#include <wdm.h>
#include <intrin.h>

// WTF? WHY DO I EVEN HAVE TO ADD THIS
#undef _NTDDK_
#include <ntddk.h>

#include "virtual_addr_helpers.h"
#include "vmcs.h"
#include "processor_context.h"
#include "msr.h"
#include "print.h"


constexpr uintptr_t kAlignmentSize = 4 * 1024;// 4KB

namespace virtualization {
	bool EnterVmxonMode()
	{
		// Set CR4.VMXE [bit 13] = 1
		// readmsr 0x3A and modify with:
		//  - Lock bit [0 bit] = 1
		//  - SMX Operation [1 bit] = 1
		//  - SMX out of operation [2 bit] = 1
		processor::ControlRegister cr0;
		processor::Cr4 cr4;
		processor::FeatureControlMsr controlMsr;
		processor::VmxBasicMsr basicMsr;
		PVOID allocatedVMX;
		ULONG64 physicalAllocatedVMX;
		unsigned char operationStatus;
		PHYSICAL_ADDRESS maxPhysical = { 0 };

		// Set CR0 Fixed values
		cr0.all = __readcr0();
		cr0.all |= __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32VmxCr0Fixed0));
		cr0.all &= __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32VmxCr0Fixed1));
		__writecr0(cr0.all);

		// Set CR4 Fixed values
		cr4.all = __readcr4();
		cr4.all |= __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32VmxCr4Fixed0));
		cr4.all &= __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32VmxCr0Fixed1));
		__writecr4(cr4.all);

		// Acquire max physical address
		AcquireMaxPhysicalAddress(maxPhysical);

		// Set CR4.VMXE = 1
		cr4.all = __readcr4();
		cr4.bitfield.vmxe = 1;
		__writecr4(cr4.all);

		// readmsr 0x3A and modify lock bit
		controlMsr.all = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32FeatureControl));

		// Lock bit = 1
		controlMsr.bitfield.lock = 1;
		controlMsr.bitfield.enable_smx = 1;
		controlMsr.bitfield.enable_vmxon = 1;

		__writemsr(static_cast<unsigned long>(msr::intel_e::kIa32FeatureControl), 
				   controlMsr.all);


		// Allocate 4KB aligned memory (not aligned yet)
		allocatedVMX = MmAllocateContiguousMemory(sizeof(VMCS) + kAlignmentSize, maxPhysical);

		if (!allocatedVMX) {
			MDbgPrint("MmAllocateContiguousMemory cannot allocate\n");
			return false;
		}
		RtlZeroMemory(allocatedVMX, sizeof(VMCS) + kAlignmentSize);

		// Align to 4KB
		allocatedVMX = reinterpret_cast<PVOID>(
			(reinterpret_cast<uintptr_t>(allocatedVMX) + kAlignmentSize - 1) & ~(kAlignmentSize - 1)
		);

		// Modify the basic revision ID
		basicMsr.all = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32VmxBasic));
		reinterpret_cast<VMCS*>(allocatedVMX)->revisionIdentifier = basicMsr.bitfield.revision_id;

		physicalAllocatedVMX = MmGetPhysicalAddress(allocatedVMX).QuadPart;
		operationStatus = __vmx_on(&physicalAllocatedVMX);
		if (operationStatus != 0) {
			MDbgPrint("Instruction VMXON failed!\n");
			return false;
		}

		return true;
	}
}