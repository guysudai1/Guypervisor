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

extern "C" void guest_code();
// extern "C" void vmexit_handler();

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
		cr0.all |= msr::ReadMsr(msr::intel_e::kIa32VmxCr0Fixed0);
		cr0.all &= msr::ReadMsr(msr::intel_e::kIa32VmxCr0Fixed1);
		__writecr0(cr0.all);

		// Set CR4 Fixed values
		cr4.all = __readcr4();
		cr4.all |= msr::ReadMsr(msr::intel_e::kIa32VmxCr4Fixed0);
		cr4.all &= msr::ReadMsr(msr::intel_e::kIa32VmxCr0Fixed1);
		__writecr4(cr4.all);

		// Set CR4.VMXE = 1
		cr4.all = __readcr4();
		cr4.fields.vmxe = 1;
		__writecr4(cr4.all);

		// readmsr 0x3A and modify lock bit
		controlMsr.all = msr::ReadMsr(msr::intel_e::kIa32FeatureControl);

		// Lock bit = 1
		controlMsr.bitfield.lock = 1;
		controlMsr.bitfield.enable_smx = 1;
		controlMsr.bitfield.enable_vmxon = 1;

		__writemsr(static_cast<unsigned long>(msr::intel_e::kIa32FeatureControl), 
				   controlMsr.all);

		// Modify the basic revision ID
		basic_msr.all = msr::ReadMsr(msr::intel_e::kIa32VmxBasic);
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
		msr_value.QuadPart = msr::ReadMsr(msr);

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
		basic_msr.all = msr::ReadMsr(msr::intel_e::kIa32VmxBasic);
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

	static void vmexit_handler() {
		NTSTATUS status = STATUS_SUCCESS;
		ExitReason exit_reason{ 0 };
		exit_reason.all = 0;

		status = ReadVMCSFieldGeneric(vmcs_field_encoding_e::kExitReason, &exit_reason.all);
		if (!NT_SUCCESS(status)) {
			MDbgPrint("Encountered an error but couldn't read exit reason from vmcs.\n");
		}

		MDbgPrint("Basic vmexit reason: %s (exit reason %X).\n",
			BasicExitReasonToString(static_cast<UINT32>(exit_reason.fields.basicExitReason)),
			exit_reason);
		MDbgPrint("Entering the guest has %s.\n", 
				(exit_reason.fields.VMEntryFailure == 1) ? "failed" : "succeeded");
		__vmx_vmresume();
	}

	void DumpVmcsGuestArea()
	{
		UINT16 value_16 = 0;
		UINT32 value_32 = 0;
		UINT64 value_64 = 0;

		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestEsSelector, &value_16);
		MDbgPrint("VMCS_GUEST_ES_SELECTOR: 0x%04X\n", value_16);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestCsSelector, &value_16);
		MDbgPrint("VMCS_GUEST_CS_SELECTOR: 0x%04X\n", value_16);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestSsSelector, &value_16);
		MDbgPrint("VMCS_GUEST_SS_SELECTOR: 0x%04X\n", value_16);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestDsSelector, &value_16);
		MDbgPrint("VMCS_GUEST_DS_SELECTOR: 0x%04X\n", value_16);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestFsSelector, &value_16);
		MDbgPrint("VMCS_GUEST_FS_SELECTOR: 0x%04X\n", value_16);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestGsSelector, &value_16);
		MDbgPrint("VMCS_GUEST_GS_SELECTOR: 0x%04X\n", value_16);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestLdtrSelector, &value_16);
		MDbgPrint("VMCS_GUEST_LDTR_SELECTOR: 0x%04X\n", value_16);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestTrSelector, &value_16);
		MDbgPrint("VMCS_GUEST_TR_SELECTOR: 0x%04X\n", value_16);
		// ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestInterruptStatus, &value_16);
		MDbgPrint("VMCS_GUEST_INTERRUPT_STATUS: 0x%04X\n", 0); // value_16));
		// ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestPmlIndex, &value_16);
		MDbgPrint("VMCS_GUEST_PML_INDEX: 0x%04X\n", 0); // value_16));

		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestPhysicalAddrFull, &value_64);
		MDbgPrint("VMCS_GUEST_PHYSICAL_ADDRESS: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestVmcsLinkPtrFull, &value_64);
		MDbgPrint("VMCS_GUEST_VMCS_LINK_POINTER: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestIa32DebugCtlFull, &value_64);
		MDbgPrint("VMCS_GUEST_DEBUGCTL: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestIa32PatFull, &value_64);
		MDbgPrint("VMCS_GUEST_PAT: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestIa32EferFull, &value_64);
		MDbgPrint("VMCS_GUEST_EFER: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestIa32PerfGlobalCtrlFull, &value_64);
		MDbgPrint("VMCS_GUEST_PERF_GLOBAL_CTRL: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestPdpte0Full, &value_64);
		MDbgPrint("VMCS_GUEST_PDPTE0: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestPdpte1Full, &value_64);
		MDbgPrint("VMCS_GUEST_PDPTE1: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestPdpte2Full, &value_64);
		MDbgPrint("VMCS_GUEST_PDPTE2: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestPdpte3Full, &value_64);
		MDbgPrint("VMCS_GUEST_PDPTE3: 0x%016llX\n", value_64);

		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestEsLimit, &value_32);
		MDbgPrint("VMCS_GUEST_ES_LIMIT: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestCsLimit, &value_32);
		MDbgPrint("VMCS_GUEST_CS_LIMIT: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestSsLimit, &value_32);
		MDbgPrint("VMCS_GUEST_SS_LIMIT: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestDsLimit, &value_32);
		MDbgPrint("VMCS_GUEST_DS_LIMIT: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestFsLimit, &value_32);
		MDbgPrint("VMCS_GUEST_FS_LIMIT: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestGsLimit, &value_32);
		MDbgPrint("VMCS_GUEST_GS_LIMIT: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestLdtrLimit, &value_32);
		MDbgPrint("VMCS_GUEST_LDTR_LIMIT: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestTrLimit, &value_32);
		MDbgPrint("VMCS_GUEST_TR_LIMIT: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestGdtrLimit, &value_32);
		MDbgPrint("VMCS_GUEST_GDTR_LIMIT: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestIdtrLimit, &value_32);
		MDbgPrint("VMCS_GUEST_IDTR_LIMIT: 0x%08lX\n", value_32);

		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestEsAccessRights, &value_32);
		MDbgPrint("VMCS_GUEST_ES_ACCESS_RIGHTS: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestCsAccessRights, &value_32);
		MDbgPrint("VMCS_GUEST_CS_ACCESS_RIGHTS: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestSsAccessRights, &value_32);
		MDbgPrint("VMCS_GUEST_SS_ACCESS_RIGHTS: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestDsAccessRights, &value_32);
		MDbgPrint("VMCS_GUEST_DS_ACCESS_RIGHTS: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestFsAccessRights, &value_32);
		MDbgPrint("VMCS_GUEST_FS_ACCESS_RIGHTS: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestGsAccessRights, &value_32);
		MDbgPrint("VMCS_GUEST_GS_ACCESS_RIGHTS: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestLdtrAccessRights, &value_32);
		MDbgPrint("VMCS_GUEST_LDTR_ACCESS_RIGHTS: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestTrAccessRights, &value_32);
		MDbgPrint("VMCS_GUEST_TR_ACCESS_RIGHTS: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestInterruptibilityState, &value_32);
		MDbgPrint("VMCS_GUEST_INTERRUPTIBILITY_STATE: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestActivityState, &value_32);
		MDbgPrint("VMCS_GUEST_ACTIVITY_STATE: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestSmbase, &value_32);
		MDbgPrint("VMCS_GUEST_SMBASE: 0x%08lX\n", value_32);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestIa32SysenterCs, &value_32);
		MDbgPrint("VMCS_GUEST_SYSENTER_CS: 0x%08lX\n", value_32);
		// ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestVmxPreemptionTimerValue, &value_32);
		MDbgPrint("VMCS_GUEST_VMX_PREEMPTION_TIMER_VALUE: 0x%08lX\n", 0); // value_32));

		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestCr0, &value_64);
		MDbgPrint("VMCS_GUEST_CR0: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestCr3, &value_64);
		MDbgPrint("VMCS_GUEST_CR3: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestCr4, &value_64);
		MDbgPrint("VMCS_GUEST_CR4: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestEsBase, &value_64);
		MDbgPrint("VMCS_GUEST_ES_BASE: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestCsBase, &value_64);
		MDbgPrint("VMCS_GUEST_CS_BASE: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestSsBase, &value_64);
		MDbgPrint("VMCS_GUEST_SS_BASE: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestDsBase, &value_64);
		MDbgPrint("VMCS_GUEST_DS_BASE: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestFsBase, &value_64);
		MDbgPrint("VMCS_GUEST_FS_BASE: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestGsBase, &value_64);
		MDbgPrint("VMCS_GUEST_GS_BASE: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestLdtrBase, &value_64);
		MDbgPrint("VMCS_GUEST_LDTR_BASE: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestTrBase, &value_64);
		MDbgPrint("VMCS_GUEST_TR_BASE: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestGdtrBase, &value_64);
		MDbgPrint("VMCS_GUEST_GDTR_BASE: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestIdtrBase, &value_64);
		MDbgPrint("VMCS_GUEST_IDTR_BASE: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestDr7, &value_64);
		MDbgPrint("VMCS_GUEST_DR7: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestRsp, &value_64);
		MDbgPrint("VMCS_GUEST_RSP: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestRip, &value_64);
		MDbgPrint("VMCS_GUEST_RIP: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestRFlags, &value_64);
		MDbgPrint("VMCS_GUEST_RFLAGS: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestPendingDebugExceptions, &value_64);
		MDbgPrint("VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestIa32SysenterEip, &value_64);
		MDbgPrint("VMCS_GUEST_SYSENTER_ESP: 0x%016llX\n", value_64);
		ReadVMCSFieldGeneric(vmcs_field_encoding::kGuestIa32SysenterEsp, &value_64);
		MDbgPrint("VMCS_GUEST_SYSENTER_EIP: 0x%016llX\n", value_64);
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
		basic_msr.all = msr::ReadMsr(msr::intel_e::kIa32VmxBasic);

		const bool consult_true_msr = basic_msr.bitfield.bit55 == 1; // Check whether to use the TRUE version of the MSRs

		/**
		 * Write VMCS link pointer
		 */
		status |= WriteVMCSField64(vmcs_field_encoding::kGuestVmcsLinkPtrFull, ~0ULL);

		MDbgPrint("Started to populate VMCS.");
		MDbgPrint("Writing Guest VMCS Link Pointer: 0x%016llX\n", ~0ULL);

		/**
		 * Write EPT Pointer
		 */
		//ept_pointer.fields.memoryType = 6; // Cache WriteBack
		//ept_pointer.fields.walkPathMinusOne = 3; // Walk length is 4 (we use a 4-level paging scheme)
		//auto pml4_address = processor_context::kProcessorContext->ept_pml4_entries;
		//auto physical_addr = MmGetPhysicalAddress(reinterpret_cast<PVOID>(pml4_address)).QuadPart;
		//ept_pointer.fields.addressPlusReserved = physical_addr;
		//ept_pointer.fields.addressPlusReserved &= AcquireMaxPhysicalAddress();
		//MDbgPrint("Pml4 table address: %X\n", pml4_address);

		//status = WriteVMCSField64(vmcs_field_encoding::kControlEPTPFull, ept_pointer.all);
		

		/**
		 *  Write Secondary Controls
		 */
		/*
		vm_secondary_ctrl_fields.fields.enableEPT = 0;
		vm_secondary_ctrl_fields.fields.enableVPID = 1;
		*/

		vm_secondary_ctrl_fields.all = ModifyControlValue(msr::intel_e::kIa32VmxProcbasedCtls2, vm_secondary_ctrl_fields.all);
		MDbgPrint("Writing Control Secondary Processor Execution Controls: 0x%08X\n", vm_secondary_ctrl_fields.all);
		status |= WriteVMCSField32(vmcs_field_encoding::kControlSecondaryProcessorVmExecutionControls, vm_secondary_ctrl_fields.all);

		/**
		 * Write VPID
		 */
		// status |= WriteVMCSField16(vmcs_field_encoding::kControlVPID, (UINT16)(KeGetCurrentProcessorNumberEx(nullptr) + 1));

		/**
		 *  Write Pin-Based Controls
		 */
		vm_pin_exec_ctrls.all = ModifyControlValue((consult_true_msr) 
													? msr::intel_e::kIa32VmxTruePinbasedCtls 
													: msr::intel_e::kIa32VmxPinbasedCtls, vm_pin_exec_ctrls.all);
		MDbgPrint("Writing Pin Based Vm Execution Controls: 0x%08X\n", vm_pin_exec_ctrls.all);
		status |= WriteVMCSField32(vmcs_field_encoding::kControlPinBasedVmExecutionControls, vm_pin_exec_ctrls.all);


		/**
		 *  Primary Control fields
		 */
		// vm_exec_ctrl_fields.fields.cr3LoadExit = 1;
		// vm_exec_ctrl_fields.fields.hltExiting = 1;
		// vm_exec_ctrl_fields.fields.rdstcExit = 1;
		// vm_exec_ctrl_fields.fields.movDrExit = 1;
		// vm_exec_ctrl_fields.fields.virtualizeMsrBitmaps = 1;
		// vm_exec_ctrl_fields.fields.virtualizeIoBitmaps = 1;
		vm_exec_ctrl_fields.fields.activateSecondaryControls = 1;
		vm_exec_ctrl_fields.all = ModifyControlValue((consult_true_msr) 
													 ? msr::intel_e::kIa32VmxTrueProcbasedCtls 
													 : msr::intel_e::kIa32VmxProcbasedCtls, vm_exec_ctrl_fields.all);
		MDbgPrint("Writing Primary Processor Vm Execution Controls: 0x%08X\n", vm_exec_ctrl_fields.all);
		status |= WriteVMCSField32(vmcs_field_encoding::kControlPrimaryProcessorVmExecutionControls, vm_exec_ctrl_fields.all);

		// Exception bitmap
		MDbgPrint("Writing Exception Bitmap: 0x%08X\n", 0);
		status |= WriteVMCSField32(vmcs_field_encoding::kControlExceptionBitmap, 0);


		/**
		 *  Exit Control Fields
		 */
		vm_exit_ctrl_fields.fields.hostAddrSpaceSize = 1;
		vm_exit_ctrl_fields.all = ModifyControlValue((consult_true_msr)
													 ? msr::intel_e::kIa32VmxTrueExitCtls
													 : msr::intel_e::kIa32VmxExitCtls, vm_exit_ctrl_fields.all);
		MDbgPrint("Writing Exit Controls: 0x%08X\n", vm_exit_ctrl_fields.all);
		status |= WriteVMCSField32(vmcs_field_encoding::kControlVmExitControls, vm_exit_ctrl_fields.all);

		/**
		 * Entry Control Fields
		 */
		
		vm_entry_ctrl_fields.fields.ia32ModeGuest = 1;
		vm_entry_ctrl_fields.all = ModifyControlValue((consult_true_msr)
													 ? msr::intel_e::kIa32VmxTrueEntryCtls 
													 : msr::intel_e::kIa32VmxEntryCtls, vm_entry_ctrl_fields.all);
		MDbgPrint("Writing Entry Controls: 0x%08X\n\n", vm_entry_ctrl_fields.all);
		status |= WriteVMCSField32(vmcs_field_encoding::kControlVmEntryControls, vm_entry_ctrl_fields.all);


		// Activity state
		status |= WriteVMCSField32(vmcs_field_encoding::kGuestActivityState, 0);

		// Interruptibility state
		status |= WriteVMCSField32(vmcs_field_encoding::kGuestInterruptibilityState, 0);

		// Debug delivery (enable / disable / ... exceptions)
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestPendingDebugExceptions, 0);

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
		status |= WriteVMCSField16(vmcs_field_encoding::kGuestCsSelector, segment_selector.cs);
		status |= WriteVMCSField32(vmcs_field_encoding::kGuestCsLimit, current_entry.fields.limit);

		// Zero out reserved bits
		// current_entry.fields.access.all &= ~(((UINT32)((1 << 4) - 1)) << 8);

		status |= WriteVMCSField32(vmcs_field_encoding::kGuestCsAccessRights, current_entry.fields.access.all);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestCsBase, current_entry.fields.base);
		// status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestCsBase, 0);
		status |= WriteVMCSField16(vmcs_field_encoding::kHostCsSelector, segment_selector.cs & ~kRplMask);

		MDbgPrint("Writing CS Segment. Host Selector: 0x%04X, Guest Selector: 0x%04X, Guest Limit: 0x%08X, Guest Access Rights: 0x%08X, Guest Base: 0x%016llX\n", 
			segment_selector.cs & ~kRplMask, segment_selector.cs, current_entry.fields.limit, current_entry.fields.access.all, current_entry.fields.base);

		/**
		 *  Load SS Segment (Ring 0 Data)
		 */
		processor::ReadGDTEntry(gdtr, segment_selector.ss, &current_entry);
		status |= WriteVMCSField16(vmcs_field_encoding::kGuestSsSelector, segment_selector.ss);
		status |= WriteVMCSField32(vmcs_field_encoding::kGuestSsLimit, current_entry.fields.limit);

		// Zero out reserved bits
		// current_entry.fields.access.all &= ~(((UINT32)((1 << 4) - 1)) << 8);

		status |= WriteVMCSField32(vmcs_field_encoding::kGuestSsAccessRights, current_entry.fields.access.all);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestSsBase, current_entry.fields.base);
		// status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestSsBase, 0);
		status |= WriteVMCSField16(vmcs_field_encoding::kHostSsSelector, segment_selector.ss & ~kRplMask);
		MDbgPrint("Writing SS Segment. Host Selector: 0x%04X, Guest Selector: 0x%04X, Guest Limit: 0x%08X, Guest Access Rights: 0x%08X, Guest Base: 0x%016llX\n",
			segment_selector.ss & ~kRplMask, segment_selector.ss, current_entry.fields.limit, current_entry.fields.access.all, current_entry.fields.base);

		/**
		 *  Load DS Segment (Ring 3 Data)
		 */
		processor::ReadGDTEntry(gdtr, segment_selector.ds, &current_entry);
		status |= WriteVMCSField16(vmcs_field_encoding::kGuestDsSelector, segment_selector.ds);
		status |= WriteVMCSField32(vmcs_field_encoding::kGuestDsLimit, current_entry.fields.limit);

		// Zero out reserved bits
		// current_entry.fields.access.all &= ~(((UINT32)((1 << 4) - 1)) << 8);

		status |= WriteVMCSField32(vmcs_field_encoding::kGuestDsAccessRights, current_entry.fields.access.all);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestDsBase, current_entry.fields.base);
		status |= WriteVMCSField16(vmcs_field_encoding::kHostDsSelector, segment_selector.ds & ~kRplMask);
		MDbgPrint("Writing DS Segment. Host Selector: 0x%04X, Guest Selector: 0x%04X, Guest Limit: 0x%08X, Guest Access Rights: 0x%08X, Guest Base: 0x%016llX\n",
			segment_selector.ds & ~kRplMask, segment_selector.ds, current_entry.fields.limit, current_entry.fields.access.all, current_entry.fields.base);

		/**
		 *  Load ES Segment (Ring 3 Data)
		 */
		processor::ReadGDTEntry(gdtr, segment_selector.es, &current_entry);
		status |= WriteVMCSField16(vmcs_field_encoding::kGuestEsSelector, segment_selector.es);
		status |= WriteVMCSField32(vmcs_field_encoding::kGuestEsLimit, current_entry.fields.limit);

		// Zero out reserved bits
		// current_entry.fields.access.all &= ~(((UINT32)((1 << 4) - 1)) << 8);

		status |= WriteVMCSField32(vmcs_field_encoding::kGuestEsAccessRights, current_entry.fields.access.all);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestEsBase, current_entry.fields.base);
		status |= WriteVMCSField16(vmcs_field_encoding::kHostEsSelector, segment_selector.es & ~kRplMask);
		MDbgPrint("Writing ES Segment. Host Selector: 0x%04X, Guest Selector: 0x%04X, Guest Limit: 0x%08X, Guest Access Rights: 0x%08X, Guest Base: 0x%016llX\n",
			segment_selector.es & ~kRplMask, segment_selector.es, current_entry.fields.limit, current_entry.fields.access.all, current_entry.fields.base);


		/**
		 *  Load FS Segment (Ring 3 Compatibility-Mode TEB)
		 */
		processor::ReadGDTEntry(gdtr, segment_selector.fs, &current_entry);
		status |= WriteVMCSField16(vmcs_field_encoding::kGuestFsSelector, segment_selector.fs);
		status |= WriteVMCSField32(vmcs_field_encoding::kGuestFsLimit, current_entry.fields.limit);

		// Zero out reserved bits
		// current_entry.fields.access.all &= ~(((UINT32)((1 << 4) - 1)) << 8);

		status |= WriteVMCSField32(vmcs_field_encoding::kGuestFsAccessRights, current_entry.fields.access.all);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestFsBase, current_entry.fields.base);
		// status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestFsBase, msr::ReadMsr(msr::intel_e::kIa32FsBase));
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostFsBase, current_entry.fields.base);
		// status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostFsBase, msr::ReadMsr(msr::intel_e::kIa32FsBase));
		status |= WriteVMCSField16(vmcs_field_encoding::kHostFsSelector, segment_selector.fs & ~kRplMask);
		/*MDbgPrint("Writing FS Segment. Host Selector: 0x%04X, Guest Selector: 0x%04X, Guest Limit: 0x%08X, Guest Access Rights: 0x%08X, Host Base: 0x%016llX, Guest Base: 0x%016llX\n",
			segment_selector.fs & ~kRplMask, segment_selector.fs, current_entry.fields.limit, current_entry.fields.access, msr::ReadMsr(msr::intel_e::kIa32FsBase), msr::ReadMsr(msr::intel_e::kIa32FsBase));*/
		MDbgPrint("Writing FS Segment. Host Selector: 0x%04X, Guest Selector: 0x%04X, Guest Limit: 0x%08X, Guest Access Rights: 0x%08X, Host Base: 0x%016llX, Guest Base: 0x%016llX\n",
			segment_selector.fs & ~kRplMask, segment_selector.fs, current_entry.fields.limit, current_entry.fields.access.all, current_entry.fields.base, current_entry.fields.base);

		/**
		 *  Load GS Segment (Ring 3 Data if in Compatibility-Mode, MSR-based in Long Mode)
		 */
		processor::ReadGDTEntry(gdtr, segment_selector.gs, &current_entry);
		status |= WriteVMCSField16(vmcs_field_encoding::kGuestGsSelector, segment_selector.gs);
		status |= WriteVMCSField32(vmcs_field_encoding::kGuestGsLimit, current_entry.fields.limit);

		// Zero out reserved bits
		// current_entry.fields.access.all &= ~(((UINT32)((1 << 4) - 1)) << 8);

		status |= WriteVMCSField32(vmcs_field_encoding::kGuestGsAccessRights, current_entry.fields.access.all);
		// status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestGsBase, current_entry.fields.base);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestGsBase, msr::ReadMsr(msr::intel_e::kIa32GsBase));
		// status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostGsBase, current_entry.fields.base);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostGsBase, msr::ReadMsr(msr::intel_e::kIa32GsBase));
		status |= WriteVMCSField16(vmcs_field_encoding::kHostGsSelector, segment_selector.gs & ~kRplMask);
		MDbgPrint("Writing GS Segment. Host Selector: 0x%04X, Guest Selector: 0x%04X, Guest Limit: 0x%08X, Guest Access Rights: 0x%08X, Host Base: 0x%016llX, Guest Base: 0x%016llX\n",
			segment_selector.gs & ~kRplMask, segment_selector.gs, current_entry.fields.limit, current_entry.fields.access.all, msr::ReadMsr(msr::intel_e::kIa32GsBase), msr::ReadMsr(msr::intel_e::kIa32GsBase));

		/**
		 *  Load TR Segment (Ring 0 TSS)
		 */
		processor::ReadGDTEntry(gdtr, segment_selector.tr, &current_entry);
		status |= WriteVMCSField16(vmcs_field_encoding::kGuestTrSelector, segment_selector.tr);
		status |= WriteVMCSField32(vmcs_field_encoding::kGuestTrLimit, current_entry.fields.limit);
		status |= WriteVMCSField32(vmcs_field_encoding::kGuestTrAccessRights, current_entry.fields.access.all);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestTrBase, current_entry.fields.base);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostTrBase, current_entry.fields.base);
		status |= WriteVMCSField16(vmcs_field_encoding::kHostTrSelector, segment_selector.tr & ~kRplMask);
		MDbgPrint("Writing TR Segment. Host Selector: 0x%04X, Guest Selector: 0x%04X, Guest Limit: 0x%08X, Guest Access Rights: 0x%08X, Host Base: 0x%016llX, Guest Base: 0x%016llX\n",
			segment_selector.tr & ~kRplMask, segment_selector.tr, current_entry.fields.limit, current_entry.fields.access.all, current_entry.fields.base, current_entry.fields.base);


		/**
		 *  Load LDTR (Ring 0 LDR)
		 */
		processor::ReadGDTEntry(gdtr, segment_selector.ldtr, &current_entry);
		status |= WriteVMCSField16(vmcs_field_encoding::kGuestLdtrSelector, segment_selector.ldtr);
		status |= WriteVMCSField32(vmcs_field_encoding::kGuestLdtrLimit, current_entry.fields.limit);
		status |= WriteVMCSField32(vmcs_field_encoding::kGuestLdtrAccessRights, current_entry.fields.access.all);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestLdtrBase, current_entry.fields.base);
		MDbgPrint("Writing LDTR Segment. Guest Selector: 0x%04X, Guest Limit: 0x%08X, Guest Access Rights: 0x%08X, Guest Base: 0x%016llX\n",
			segment_selector.ldtr, current_entry.fields.limit, current_entry.fields.access.all, current_entry.fields.base);


		/**
		 *  Load GDTR 
		 */
		status |= WriteVMCSField32(vmcs_field_encoding::kGuestGdtrLimit, gdtr.limit);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestGdtrBase, gdtr.base);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostGdtrBase, gdtr.base);
		MDbgPrint("Writing GDTR Segment. Guest Limit: 0x%08X, Host Base: 0x%016llX, Guest Base: 0x%016llX\n",
			gdtr.limit, gdtr.base, gdtr.base);

		/**
		 *  Load IDTR
		 */
		status |= WriteVMCSField32(vmcs_field_encoding::kGuestIdtrLimit, idtr.limit);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestIdtrBase, idtr.base);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostIdtrBase, idtr.base);
		MDbgPrint("Writing IDTR Segment. Guest Limit: 0x%08X, Host Base: 0x%016llX, Guest Base: 0x%016llX\n\n",
			idtr.limit, idtr.base, idtr.base);

		// 
		// Register Loading Stage
		//

		/**
		 * Load CR0
		 */
		auto cr0 = __readcr0();
		cr0 |= msr::ReadMsr(msr::intel_e::kIa32VmxCr0Fixed0);
		cr0 &= msr::ReadMsr(msr::intel_e::kIa32VmxCr0Fixed1);

		// Set CR4 Fixed values
		auto cr4 = __readcr4();
		cr4 = __readcr4();
		cr4 |= msr::ReadMsr(msr::intel_e::kIa32VmxCr4Fixed0);
		cr4 &= msr::ReadMsr(msr::intel_e::kIa32VmxCr0Fixed1);

		processor::Cr0 guest_cr0_shadow{ 0 };
		processor::Cr4 guest_cr4_shadow{ 0 };

		processor::Cr0 guest_cr0_mask{ 0 };
		processor::Cr4 guest_cr4_mask{ 0 };

		guest_cr0_shadow.all = cr0;
		guest_cr4_shadow.all = cr4;

		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestCr0, cr0);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostCr0, __readcr0());
		MDbgPrint("Writing Guest CR0: 0x%016llX, Host CR0: 0x%016llX\n", cr0, __readcr0());

		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kControlCr0Mask, guest_cr0_mask.all);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kControlCr0ReadShadow, guest_cr0_shadow.all);
		MDbgPrint("Writing CR0 Mask: 0x%016llX, CR0 Shadow: 0x%016llX\n", guest_cr0_mask.all, guest_cr0_shadow.all);

		/**
		 * Load CR3
		 */
		const auto cr3 = __readcr3();
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestCr3, cr3);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostCr3, cr3);
		MDbgPrint("Writing Guest CR3: 0x%016llX, Host CR3: 0x%016llX\n", cr3, cr3);

		/**
		 * Load CR4
		 */
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestCr4, cr4);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostCr4, __readcr4());
		MDbgPrint("Writing Guest CR4: 0x%016llX, Host CR4: 0x%016llX\n", cr4, __readcr4());

		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kControlCr4Mask, guest_cr4_mask.all);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kControlCr4ReadShadow, guest_cr4_shadow.all);
		MDbgPrint("Writing CR4 Mask: 0x%016llX, CR4 Shadow: 0x%016llX\n\n", guest_cr4_mask.all, guest_cr4_shadow.all);

		/*
		 *  Load debug MSR and DR7 register
		 */
		// status |= WriteVMCSField64(vmcs_field_encoding::kGuestIa32DebugCtlFull, msr::ReadMsr(msr::intel_e::kIa32DebugCtl));
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestDr7, __readdr(7));

		// MDbgPrint("Writing Guest IA32_DEBUG_CTL: 0x%016llX\n", msr::ReadMsr(msr::intel_e::kIa32DebugCtl));
		MDbgPrint("Writing Guest DR7: 0x%016llX\n\n", __readdr(7));

		/*
		 * Load Guest Context (RSP, RIP, RFLAGS)
		 */

		char guest_stack[4096] = { 0 };
		char host_stack[4096] = { 0 };
		auto rflags = __readeflags();
		// rflags &= (~(unsigned long long)(1 << 9));

		// status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestRsp, (uintptr_t)(&processor_context::kProcessorContext->guest_stack) + kStackSize - 8);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestRsp, (uintptr_t)&guest_stack + sizeof(guest_stack) - 8);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestRip, (unsigned long long)(&guest_code));
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestRFlags, rflags);
		MDbgPrint("Writing Guest RSP: 0x%016llX\n", (uintptr_t)&guest_stack + sizeof(guest_stack) - 8);
		MDbgPrint("Writing Guest RIP: 0x%016llX\n", (unsigned long long)(&guest_code));
		MDbgPrint("Writing Guest RFlags: 0x%016llX\n\n", rflags);


		/** 
		 * Load Host Context (RSP, RIP)
		 */

#ifdef __64BIT__
		// status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostRsp, (uintptr_t)(&processor_context::kProcessorContext->host_stack) + kStackSize - 8);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostRsp, (uintptr_t)&host_stack + sizeof(host_stack) - 8);
#else
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostRsp, __read_esp());
#endif
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostRip, (unsigned long long)(&vmexit_handler));
		MDbgPrint("Writing Host RSP: 0x%016llX\n", (uintptr_t)&host_stack + sizeof(host_stack) - 8);
		MDbgPrint("Writing Host RIP (VMExit Handler): 0x%016llX\n\n", (unsigned long long)(&vmexit_handler));


		UINT64 sysenter_cs_msr = msr::ReadMsr(msr::intel_e::kIa32SysenterCs);
		processor::natural_width sysenter_esp_msr = msr::ReadMsr(msr::intel_e::kIa32SysenterEsp);
		processor::natural_width sysenter_eip_msr = msr::ReadMsr(msr::intel_e::kIa32SysenterEip);

		status |= WriteVMCSField32(vmcs_field_encoding::kHostIa32SysenterCs, (UINT32)sysenter_cs_msr);
		status |= WriteVMCSField32(vmcs_field_encoding::kGuestIa32SysenterCs, (UINT32)sysenter_cs_msr);
		MDbgPrint("Writing Guest IA32_SYSENTER_CS: 0x%08X\n", (UINT32)sysenter_cs_msr);
		MDbgPrint("Writing Host IA32_SYSENTER_CS: 0x%08X\n\n", (UINT32)sysenter_cs_msr);

		
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestIa32SysenterEsp, sysenter_esp_msr);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kGuestIa32SysenterEip, sysenter_eip_msr);
		MDbgPrint("Writing Guest IA32_SYSENTER_ESP: 0x%08X\n", (UINT32)sysenter_esp_msr);
		MDbgPrint("Writing Guest IA32_SYSENTER_EIP: 0x%08X\n\n", (UINT32)sysenter_eip_msr);

		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostIa32SysenterEsp, sysenter_esp_msr);
		status |= WriteVMCSFieldNatural(vmcs_field_encoding::kHostIa32SysenterEip, sysenter_eip_msr);
		MDbgPrint("Writing Host IA32_SYSENTER_ESP: 0x%08X\n", (UINT32)sysenter_esp_msr);
		MDbgPrint("Writing Host IA32_SYSENTER_EIP: 0x%08X\n\n", (UINT32)sysenter_eip_msr);

		/*
		msr::VmxMiscMsr misc_msr{ 0 };
		misc_msr.all = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32VmxMisc));

		// Debug delivery (enable / disable / ... exceptions)
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestPendingDebugExceptions, 0x00000000);

		MDbgPrint("Cr3 target values supported: %X\n", misc_msr.fields.cr3_target_values_supported);
		status = WriteVMCSField32(vmcs_field_encoding::kControlCr3TargetCount, misc_msr.fields.cr3_target_values_supported);
		*/

		DumpVmcsGuestArea();

		if (status != STATUS_SUCCESS) {
			MDbgPrint("Something went wrong...\n");
		}
		return status;
	}

	NTSTATUS LaunchGuest() {
		NTSTATUS status = STATUS_SUCCESS;
		unsigned char operationStatus = 0;

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

		status = ReadVMCSFieldGeneric(vmcs_field_encoding_e::kVmInstructionError, &instruction_error);

		if (!NT_SUCCESS(status)) {
			MDbgPrint("Encountered an error but couldn't read instruction error from vmcs.\n");
			goto cleanup;
		}
		
		status = ReadVMCSFieldGeneric(vmcs_field_encoding_e::kExitReason, &exit_reason.all);
		if (!NT_SUCCESS(status)) {
			MDbgPrint("Encountered an error but couldn't read exit reason from vmcs.\n");
			goto cleanup;
		}

		MDbgPrint("Encountered an error: %s (status code %ld).\n", 
			InstructionErrorToString(static_cast<UINT32>(instruction_error)), 
			instruction_error);
		MDbgPrint("Basic exit reason: %s (exit reason %X).\n",
			BasicExitReasonToString(static_cast<UINT32>(exit_reason.fields.basicExitReason)),
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

	template <class T>
	NTSTATUS ReadVMCSFieldGeneric(vmcs_field_encoding_e encoding, T	* field) {
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

		*field = static_cast<T>(tmp);

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
		status = ReadVMCSFieldGeneric(encoding, field);
#endif

		return status;
	}
}