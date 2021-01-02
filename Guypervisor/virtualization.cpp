#include "virtualization.h"

#include <wdm.h>
#include <intrin.h>

// WTF? WHY DO I EVEN HAVE TO ADD THIS
#undef _NTDDK_
#include <ntddk.h>

#include "vmcs.h"
#include "processor_context.h"
#include "msr.h"
#include "print.h"
#include "custom_status_codes.h"
#include "virtual_addr_helpers.h"
#include "vmcs_field_index.h"
#include "error_codes.h"
#include "processor_context.h"

namespace virtualization {
	NTSTATUS EnterVmxonMode()
	{
		// Set CR4.VMXE [bit 13] = 1
		// readmsr 0x3A and modify with:
		//  - Lock bit [0 bit] = 1
		//  - SMX Operation [1 bit] = 1
		//  - SMX out of operation [2 bit] = 1
		NTSTATUS status = STATUS_SUCCESS;

		processor::Cr0 cr0{0};
		processor::Cr4 cr4{0};
		msr::FeatureControlMsr controlMsr{0};
		msr::VmxBasicMsr basic_msr{0};
		ULONG64 physicalAllocatedVMX;
		unsigned char operationStatus = 0;
		processor_context::processorContext* context = processor_context::kProcessorContext;

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

		// Set CR4.VMXE = 1
		cr4.all = __readcr4();
		cr4.fields.vmxe = 1;
		__writecr4(cr4.all);

		// readmsr 0x3A and modify lock bit
		controlMsr.all = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32FeatureControl));

		// Lock bit = 1
		controlMsr.bitfield.lock = 1;
		controlMsr.bitfield.enable_smx = 1;
		controlMsr.bitfield.enable_vmxon = 1;

		__writemsr(static_cast<unsigned long>(msr::intel_e::kIa32FeatureControl), 
				   controlMsr.all);

		// Modify the basic revision ID
		basic_msr.all = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32VmxBasic));
		context->vmxon_region->revisionIdentifier = basic_msr.fields.revision_id;

		physicalAllocatedVMX = MmGetPhysicalAddress(context->vmxon_region).QuadPart;
		operationStatus = __vmx_on(&physicalAllocatedVMX);
		if (operationStatus != 0)
		{
			status = STATUS_FAILED_VMXON;
			goto cleanup;
		}

	cleanup:
		return status;
	}

	UINT32 ModifyControlValue(msr::intel_e msr, UINT32 requested_value) {
		LARGE_INTEGER msr_value = {};
		msr_value.QuadPart = __readmsr(static_cast<unsigned long>(msr));

		// 0 In high word means can't be zero 
		requested_value &= msr_value.HighPart;
		// 1 In low word means must be one
		requested_value |= msr_value.LowPart;
		return requested_value;
	}

	NTSTATUS InitializeVMCS()
	{
		NTSTATUS status = STATUS_SUCCESS;
		ULONG64 physicalAllocatedVMX;
		msr::VmxBasicMsr basic_msr{ 0 };
		processor_context::processorContext* context = processor_context::kProcessorContext;
		unsigned char operationStatus = 0;

		// Modify the basic revision ID
		basic_msr.all = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32VmxBasic));
		context->vmcs_region->revisionIdentifier = basic_msr.fields.revision_id;

		physicalAllocatedVMX = MmGetPhysicalAddress(context->vmcs_region).QuadPart;
		operationStatus = __vmx_vmclear(&physicalAllocatedVMX);

		if (operationStatus != 0)
		{
			status = STATUS_FAILED_VMCLEAR;
			goto cleanup;
		}

		operationStatus = __vmx_vmptrld(&physicalAllocatedVMX);

		if (operationStatus != 0)
		{
			status = STATUS_FAILED_VMPTRLD;
			goto cleanup;
		}

	cleanup:
		return status;
	}
	static void test_runner_handler() {
		MDbgPrint("Testing this\n");
	}

	static void vmexit_handler() {
		MDbgPrint("OH NO! THEY USED VMEXIT\n");
	}

	NTSTATUS PopulateActiveVMCS() {
		NTSTATUS status = STATUS_SUCCESS;

		PinBasedControls vm_pin_exec_ctrls{ 0 };
		VMExecCtrlFields vm_exec_ctrl_fields{ 0 };
		SecondaryVMExecCtrls vm_secondary_ctrl_fields{ 0 };
		VMExitCtrlFields vm_exit_ctrl_fields{ 0 };
		VMEntryCtrlFields vm_entry_ctrl_fields{ 0 };
		EPTP ept_pointer{ 0 };

		msr::VmxBasicMsr basic_msr{ 0 };
		basic_msr.all = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32VmxBasic));

		const bool consult_true_msr = basic_msr.bitfield.bit55 == 1; // Check whether to use the TRUE version of the MSRs

		/**
		 * Write VMCS link pointer
		 */
		status = WriteVMCSField64(vmcs_field_encoding::kGuestVmcsLinkPtrFull, ~0ULL);

		/**
		 * Write EPT Pointer
		 */
		//ept_pointer.fields.memoryType = 6; // Cache WriteBack
		//ept_pointer.fields.walkPathMinusOne = 3; // Walk length is 4 (we use a 4-level paging scheme)
		//auto pml4_address = processor_context::kProcessorContext->ept_pml4_entries;
		//auto physical_addr = MmGetPhysicalAddress(reinterpret_cast<PVOID>(pml4_address)).QuadPart / PAGE_SIZE;
		//ept_pointer.fields.addressPlusReserved = physical_addr;
		//ept_pointer.fields.addressPlusReserved &= AcquireMaxPhysicalAddress();
		//MDbgPrint("Pml4 table address: %X\n", pml4_address);

		//status = WriteVMCSField64(vmcs_field_encoding::kControlEPTPFull, ept_pointer.all);
		status = WriteVMCSField16(vmcs_field_encoding::kControlVPID, 1);

		/**
		 *  Write Secondary Controls
		 */
		// vm_secondary_ctrl_fields.fields.enableEPT = 1;
		vm_secondary_ctrl_fields.fields.enableVPID = 1;
		vm_secondary_ctrl_fields.fields.enableINVPCID = 1;
		vm_secondary_ctrl_fields.fields.enableXSaves_XStores = 1;
		vm_secondary_ctrl_fields.fields.enableRdtscp = 1;
		vm_secondary_ctrl_fields.all = ModifyControlValue(msr::intel_e::kIa32VmxProcbasedCtls2, vm_secondary_ctrl_fields.all);
		status = WriteVMCSField32(vmcs_field_encoding::kControlSecondaryProcessorVmExecutionControls, vm_secondary_ctrl_fields.all);

		/**
		 *  Write Pin-Based Controls
		 */
		vm_pin_exec_ctrls.all = ModifyControlValue((consult_true_msr) 
													? msr::intel_e::kIa32VmxTruePinbasedCtls 
													: msr::intel_e::kIa32VmxPinbasedCtls, vm_pin_exec_ctrls.all);
		status = WriteVMCSField32(vmcs_field_encoding::kControlPinBasedVmExecutionControls, vm_pin_exec_ctrls.all);


		/**
		 *  Primary Control fields
		 */
		vm_exec_ctrl_fields.fields.secondControls = 1;
		vm_exec_ctrl_fields.fields.virtualizeMsrBitmaps = 1;
		vm_exec_ctrl_fields.all = ModifyControlValue((consult_true_msr) 
													 ? msr::intel_e::kIa32VmxTrueProcbasedCtls 
													 : msr::intel_e::kIa32VmxProcbasedCtls, vm_exec_ctrl_fields.all);
		status = WriteVMCSField32(vmcs_field_encoding::kControlPrimaryProcessorVmExecutionControls, vm_exec_ctrl_fields.all);

		/**
		 *  Exit Control Fields
		 */
		vm_exit_ctrl_fields.fields.hostAddrSpaceSize = 1;
		vm_exit_ctrl_fields.all = ModifyControlValue((consult_true_msr)
													 ? msr::intel_e::kIa32VmxTrueExitCtls
													 : msr::intel_e::kIa32VmxExitCtls, vm_exit_ctrl_fields.all);
		status = WriteVMCSField32(vmcs_field_encoding::kControlVmExitControls, vm_exit_ctrl_fields.all);

		/**
		 * Entry Control Fields
		 */
		
		vm_entry_ctrl_fields.fields.ia32ModeGuest = 1;
		vm_entry_ctrl_fields.all = ModifyControlValue((consult_true_msr)
													 ? msr::intel_e::kIa32VmxTrueEntryCtls 
													 : msr::intel_e::kIa32VmxEntryCtls, vm_entry_ctrl_fields.all);
		status = WriteVMCSField32(vmcs_field_encoding::kControlVmEntryControls, vm_entry_ctrl_fields.all);


		processor::segmentSelector segment_selector{ 0 };
		processor::GDTR gdtr = { 0 };
		processor::IDTR idtr = { 0 };
		processor::vmxGdtEntry current_entry{ 0 };

		processor::LoadSegmentSelectors(&segment_selector);
		_sgdt(&gdtr);
		__sidt(&idtr);
		
		/**
		 *  Load CS Segment (Ring 0 Code)
		 */
		processor::ReadGDTEntry(gdtr, segment_selector.cs, &current_entry);
		status = WriteVMCSField16(vmcs_field_encoding::kGuestCsSelector, segment_selector.cs);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestCsLimit, current_entry.fields.limit);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestCsAccessRights, current_entry.fields.access);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestCsBase, current_entry.fields.base);
		status = WriteVMCSField16(vmcs_field_encoding::kHostCsSelector, segment_selector.cs & ~kRplMask);

		/**
		 *  Load SS Segment (Ring 0 Data)
		 */
		processor::ReadGDTEntry(gdtr, segment_selector.ss, &current_entry);
		status = WriteVMCSField16(vmcs_field_encoding::kGuestSsSelector, segment_selector.ss);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestSsLimit, current_entry.fields.limit);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestSsAccessRights, current_entry.fields.access);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestSsBase, current_entry.fields.base);
		status = WriteVMCSField16(vmcs_field_encoding::kHostSsSelector, segment_selector.ss & ~kRplMask);

		/**
		 *  Load DS Segment (Ring 3 Data)
		 */
		processor::ReadGDTEntry(gdtr, segment_selector.ds, &current_entry);
		status = WriteVMCSField16(vmcs_field_encoding::kGuestDsSelector, segment_selector.ds);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestDsLimit, current_entry.fields.limit);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestDsAccessRights, current_entry.fields.access);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestDsBase, current_entry.fields.base);
		status = WriteVMCSField16(vmcs_field_encoding::kHostDsSelector, segment_selector.ds & ~kRplMask);

		/**
		 *  Load ES Segment (Ring 3 Data)
		 */
		processor::ReadGDTEntry(gdtr, segment_selector.es, &current_entry);
		status = WriteVMCSField16(vmcs_field_encoding::kGuestEsSelector, segment_selector.es);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestEsLimit, current_entry.fields.limit);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestEsAccessRights, current_entry.fields.access);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestEsBase, current_entry.fields.base);
		status = WriteVMCSField16(vmcs_field_encoding::kHostEsSelector, segment_selector.es & ~kRplMask);


		/**
		 *  Load FS Segment (Ring 3 Compatibility-Mode TEB)
		 */
		processor::ReadGDTEntry(gdtr, segment_selector.fs, &current_entry);
		status = WriteVMCSField16(vmcs_field_encoding::kGuestFsSelector, segment_selector.fs);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestFsLimit, current_entry.fields.access);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestFsAccessRights, current_entry.fields.access);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestFsBase, current_entry.fields.base);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostFsBase, current_entry.fields.base);
		status = WriteVMCSField16(vmcs_field_encoding::kHostFsSelector, segment_selector.fs & ~kRplMask);

		/**
		 *  Load GS Segment (Ring 3 Data if in Compatibility-Mode, MSR-based in Long Mode)
		 */
		processor::ReadGDTEntry(gdtr, segment_selector.gs, &current_entry);
		status = WriteVMCSField16(vmcs_field_encoding::kGuestGsSelector, segment_selector.gs);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestGsLimit, current_entry.fields.access);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestGsAccessRights, current_entry.fields.access);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestGsBase, current_entry.fields.base);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostGsBase, current_entry.fields.base);
		status = WriteVMCSField16(vmcs_field_encoding::kHostGsSelector, segment_selector.gs & ~kRplMask);

		/**
		 *  Load TR Segment (Ring 0 TSS)
		 */
		processor::ReadGDTEntry(gdtr, segment_selector.tr, &current_entry);
		status = WriteVMCSField16(vmcs_field_encoding::kGuestTrSelector, segment_selector.tr);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestTrLimit, current_entry.fields.limit);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestTrAccessRights, current_entry.fields.access);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestTrBase, current_entry.fields.base);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostTrBase, current_entry.fields.base);
		status = WriteVMCSField16(vmcs_field_encoding::kHostTrSelector, segment_selector.tr & ~kRplMask);


		/**
		 *  Load LDTR (Ring 0 LDR)
		 */
		processor::ReadGDTEntry(gdtr, segment_selector.ldtr, &current_entry);
		status = WriteVMCSField16(vmcs_field_encoding::kGuestLdtrSelector, segment_selector.ldtr);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestLdtrLimit, current_entry.fields.limit);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestLdtrAccessRights, current_entry.fields.access);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestLdtrBase, current_entry.fields.base);


		/**
		 *  Load GDTR 
		 */
		status = WriteVMCSField32(vmcs_field_encoding::kGuestGdtrLimit, gdtr.limit);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestGdtrBase, gdtr.base);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostGdtrBase, gdtr.base);

		/**
		 *  Load IDTR
		 */
		status = WriteVMCSField32(vmcs_field_encoding::kGuestIdtrLimit, idtr.limit);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestIdtrBase, idtr.base);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostIdtrBase, idtr.base);

		// 
		// Register Loading Stage
		//
		processor::Cr0 new_guest_cr0{ 0 };
		processor::Cr3 new_guest_cr3{ 0 };
		processor::Cr4 new_guest_cr4{ 0 };

		new_guest_cr0.all = __readcr0();
		new_guest_cr3.all = __readcr3();
		new_guest_cr4.all = __readcr4();

		/**
		 * Load CR0
		 */
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestCr0, new_guest_cr0.all);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kControlCr0ReadShadow, new_guest_cr0.all);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostCr0, new_guest_cr0.all);

		/**
		 * Load CR3
		 */
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestCr3, new_guest_cr3.all);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostCr0, new_guest_cr3.all);

		/**
		 * Load CR4
		 */
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestCr4, new_guest_cr4.all);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostCr4, new_guest_cr4.all);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kControlCr4ReadShadow, new_guest_cr4.all);


		/*
		 *  Load debug MSR and DR7 register
		 */
		status = WriteVMCSField64(vmcs_field_encoding::kGuestIa32DebugCtlFull, __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32DebugCtl)));
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestDr7, __readdr(7));

		/*
		 * Load Guest Context (RSP, RIP, RFLAGS)
		 */
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestRsp, (uintptr_t)processor_context::kProcessorContext->guest_stack + kStackSize - 8);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestRip, reinterpret_cast<UINT64>(&test_runner_handler));
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestRFlags, __readeflags());

		/** 
		 * Load Host Context (RSP, RIP)
		 */
#ifdef __64BIT__
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostRsp, __read_rsp());
#else
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostRsp, __read_esp());
#endif
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostRip, reinterpret_cast<processor::natural_width>(&vmexit_handler));

		/*
		msr::VmxMiscMsr misc_msr{ 0 };
		misc_msr.all = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32VmxMisc));

		// processor::natural_width debugctl_msr = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32DebugCtl));
		processor::natural_width sysenter_esp_msr = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32SysenterEsp));
		processor::natural_width sysenter_eip_msr = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32SysenterEip)); 
		UINT64 sysenter_cs_msr = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32SysenterCs));



		
		/**
		 *  Write Guest fields (filled using the RESET table in page 3110)
		 

		// Control registers
		

		// Guest states
		status = WriteVMCSField32(vmcs_field_encoding::kGuestInterruptibilityState, 0x00000000);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestActivityState, 0x00000000);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestSmbase, 0x00030000);

		// Debug delivery (enable / disable / ... exceptions)
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestPendingDebugExceptions, 0x00000000);

		// MSRs
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestIa32SysenterEsp,	0);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestIa32SysenterEip,	0);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestIa32SysenterCs,		0);
		

		// Link ptr write
		

		/**
		 *  Write host fields
		 

		// TODO: Add tr base?
		// status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostTrBase, );
		
		// MSRs
		status = WriteVMCSField32(vmcs_field_encoding::kHostIa32SysenterCs, static_cast<UINT32>(sysenter_cs_msr));
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostIa32SysenterEsp, sysenter_esp_msr);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostIa32SysenterEip, sysenter_eip_msr);
		
		// Registers

		// Control Registers
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostCr0, __readcr0());
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostCr3, __readcr3());
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostCr4, __readcr4());

		/**
		 *  Control fields
		 

		
	
		MDbgPrint("Cr3 target values supported: %X\n", misc_msr.fields.cr3_target_values_supported);
		status = WriteVMCSField32(vmcs_field_encoding::kControlCr3TargetCount, misc_msr.fields.cr3_target_values_supported);
		*/

	// cleanup:
		return status;
	}

	NTSTATUS LaunchGuest() {
		NTSTATUS status = STATUS_SUCCESS;
		unsigned char operationStatus = 0;

		// TODO: Actually create the vmlaunch function
		operationStatus = __vmx_vmlaunch();

		if (operationStatus != 0)
		{
			status = STATUS_FAILED_VMLAUNCH;
			PrintVMXError();
			goto cleanup;
		}

	cleanup:
		return status;
	}

	NTSTATUS PrintVMXError() {
		NTSTATUS status = STATUS_SUCCESS;
		UINT32 instruction_error = 0;
		ExitReason exit_reason{0};
		exit_reason.all = 0;

		status = ReadVMCSField32(vmcs_field_encoding_e::kVmInstructionError, &instruction_error);

		if (!NT_SUCCESS(status)) {
			MDbgPrint("Encountered an error but couldn't read instruction error from vmcs.\n");
			goto cleanup;
		}
		
		status = ReadVMCSField32(vmcs_field_encoding_e::kExitReason, &exit_reason.all);
		if (!NT_SUCCESS(status)) {
			MDbgPrint("Encountered an error but couldn't read exit reason from vmcs.\n");
			goto cleanup;
		}

		MDbgPrint("Encountered an error: %s (status code %ld).\n", 
			InstructionErrorToString(static_cast<UINT32>(instruction_error)), 
			instruction_error);
		MDbgPrint("Basic exit reason: %s (exit reason %X).\n",
			BasicExitReasonToString(static_cast<UINT32>(exit_reason.all)),
			exit_reason);

	cleanup:
		return status;
	}
	
	NTSTATUS ClearActiveVMCS() {
		NTSTATUS status = STATUS_SUCCESS;
		UINT64 vmcs_physical_addr = NULL;
		unsigned int operation_status = 0;

		__vmx_vmptrst(&vmcs_physical_addr);
		if (vmcs_physical_addr == -1) {
			PrintVMXError();
			status = STATUS_FAILED_VMPTRLD;
			goto cleanup;
		}

		operation_status = __vmx_vmclear(&vmcs_physical_addr);
		if (operation_status != 0) {
			if (operation_status == 1) {
				PrintVMXError();
			}
			status = STATUS_FAILED_VMCLEAR;
			goto cleanup;
		}

	cleanup:
		return status;
	}

	/**
	 * VMX VMWrite functions (64 bit, 32 bit, natural width)
	 */

	NTSTATUS WriteVMCSFieldNatural(vmcs_field_encoding_e encoding,
		processor::natural_width value) {
		NTSTATUS status = STATUS_SUCCESS;

#ifdef __64BIT__
		status = WriteVMCSField64(encoding, value);
#else
		status = WriteVMCSField32(encoding, value);
#endif

		return status;
	}

	NTSTATUS WriteVMCSField16(vmcs_field_encoding_e encoding,
							  UINT16 value) {
		NTSTATUS status = STATUS_SUCCESS;

		status = WriteVMCSField64(encoding, static_cast<UINT64>(value));

		if (!NT_SUCCESS(status)) {
			goto cleanup;
		}

	cleanup:
		return status;
	}

	NTSTATUS WriteVMCSField32(vmcs_field_encoding_e encoding,
							  UINT32 value) {
		NTSTATUS status = STATUS_SUCCESS;

		status = WriteVMCSField64(encoding, static_cast<UINT64>(value));

		if (!NT_SUCCESS(status)) {
			goto cleanup;
		}

	cleanup:
		return status;
	}

	NTSTATUS WriteVMCSField64(vmcs_field_encoding_e encoding,
							  UINT64 value) {
		NTSTATUS status = STATUS_SUCCESS;

		unsigned char operationStatus = 0;

		operationStatus = __vmx_vmwrite(encoding, value);
		if (operationStatus != 0) {
			status = STATUS_FAILED_VMWRITE;
			PrintVMXError();
			goto cleanup;
		}

	cleanup:
		return status;
	}

	/**
	 * VMX VMRead functions (64 bit, 32 bit, natural width)
	 */

	NTSTATUS ReadVMCSField32(vmcs_field_encoding_e encoding,
							 UINT32* field) {
		NTSTATUS status = STATUS_SUCCESS;
		SIZE_T tmp = 0;

		if (field == nullptr) {
			status = STATUS_NULLPTR_ERROR;
			goto cleanup;
		}

		status = ReadVMCSField64(encoding, &tmp);

		if (!NT_SUCCESS(status)) {
			goto cleanup;
		}

		*field = static_cast<UINT32>(tmp);

	cleanup:
		return status;
	}

	NTSTATUS ReadVMCSField64(vmcs_field_encoding_e encoding,
						     UINT64* field) {
		NTSTATUS status = STATUS_SUCCESS;

		unsigned char operationStatus = 0;

		if (field == nullptr) {
			status = STATUS_NULLPTR_ERROR;
			goto cleanup;
		}

		operationStatus = __vmx_vmread(encoding, field);
		if (operationStatus != 0) {
			status = STATUS_FAILED_VMREAD;
			PrintVMXError();
			goto cleanup;
		}

	cleanup:
		return status;
	}

	NTSTATUS ReadVMCSFieldNatural(vmcs_field_encoding_e encoding,
								  processor::natural_width* field) {
		NTSTATUS status = STATUS_SUCCESS;

#ifdef __64BIT__
		status = ReadVMCSField64(encoding, field);
#else
		status = ReadVMCSField32(encoding, field);
#endif

		return status;
	}
}