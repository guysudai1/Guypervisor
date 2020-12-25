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
		processor::FeatureControlMsr controlMsr{0};
		processor::VmxBasicMsr basicMsr{0};
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
		basicMsr.all = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32VmxBasic));
		context->vmxon_region->revisionIdentifier = basicMsr.bitfield.revision_id;

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

	NTSTATUS InitializeVMCS()
	{
		NTSTATUS status = STATUS_SUCCESS;
		ULONG64 physicalAllocatedVMX;
		processor::VmxBasicMsr basicMsr{ 0 };
		processor_context::processorContext* context = processor_context::kProcessorContext;
		unsigned char operationStatus = 0;

		// Modify the basic revision ID
		basicMsr.all = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32VmxBasic));
		context->vmcs_region->revisionIdentifier = basicMsr.bitfield.revision_id;

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

	NTSTATUS PopulateActiveVMCS() {
		NTSTATUS status = STATUS_SUCCESS;

		PinBasedControls vm_pin_exec_ctrls{ 0 };
		VMExecCtrlFields vm_exec_ctrl_fields{ 0 };
		SecondaryVMExecCtrls vm_secondary_ctrl_fields{ 0 };


		processor::Cr0 new_guest_cr0{0};
		processor::Cr3 new_guest_cr3{0};
		processor::Cr4 new_guest_cr4{0};

		processor::segmentSelector segment_selector{ 0 };
		// processor::natural_width debugctl_msr = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32DebugCtl));
		processor::natural_width sysenter_esp_msr = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32SysenterEsp));
		processor::natural_width sysenter_eip_msr = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32SysenterEip)); 
		UINT64 sysenter_cs_msr = __readmsr(static_cast<unsigned long>(msr::intel_e::kIa32SysenterCs));

		new_guest_cr0.all = 0x60000010;
		new_guest_cr3.all = 0;
		new_guest_cr4.all = 0;

		processor::LoadSegmentSelectors(&segment_selector);

		/**
		 *  Write Guest fields (filled using the RESET table in page 3110)
		 */

		// Selectors
		status = WriteVMCSField16(vmcs_field_encoding::kGuestEsSelector, 0x0000);
		status = WriteVMCSField16(vmcs_field_encoding::kGuestCsSelector, 0xF000);
		status = WriteVMCSField16(vmcs_field_encoding::kGuestSsSelector, 0x0000);
		status = WriteVMCSField16(vmcs_field_encoding::kGuestDsSelector, 0x0000);
		status = WriteVMCSField16(vmcs_field_encoding::kGuestFsSelector, 0x0000);
		status = WriteVMCSField16(vmcs_field_encoding::kGuestGsSelector, 0x0000);
		status = WriteVMCSField16(vmcs_field_encoding::kGuestLdtrSelector, 0x0000);
		status = WriteVMCSField16(vmcs_field_encoding::kGuestTrSelector, 0x0000);

		// Limit
		status = WriteVMCSField32(vmcs_field_encoding::kGuestEsLimit,	0xFFFF);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestCsLimit,	0xFFFF);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestSsLimit,	0xFFFF);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestDsLimit,	0xFFFF);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestFsLimit,	0xFFFF);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestGsLimit,	0xFFFF);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestLdtrLimit, 0xFFFF);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestTrLimit,	0xFFFF);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestGdtrLimit, 0xFFFF);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestIdtrLimit, 0xFFFF);

		// Access Rights
		status = WriteVMCSField32(vmcs_field_encoding::kGuestEsAccessRights,	0x0093);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestCsAccessRights,	0x0093);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestSsAccessRights,	0x0093);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestDsAccessRights,	0x0093);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestFsAccessRights,	0x0093);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestGsAccessRights,	0x0093);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestLdtrAccessRights,	0x0082);
		status = WriteVMCSField32(vmcs_field_encoding::kGuestTrAccessRights,	0x0082);

		// Base 
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestEsBase,	0x00000000);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestCsBase,	0xFFFF0000);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestSsBase,	0x00000000);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestDsBase,	0x00000000);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestFsBase,	0x00000000);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestGsBase,	0x00000000);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestLdtrBase, 0x00000000);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestTrBase,	0x00000000);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestGdtrBase, 0x00000000);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestIdtrBase, 0x00000000);

		// Registers
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestDr7, 0x00000400);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestRsp, 0x00000000);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestRip, 0x0000FFF0);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestRFlags, 0x00000002);

		// Control registers
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestCr0, new_guest_cr0.all);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestCr3, new_guest_cr3.all);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kGuestCr4, new_guest_cr4.all);

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
		status = WriteVMCSField64(vmcs_field_encoding::kGuestIa32DebugCtlFull,		0);

		// Link ptr write
		status = WriteVMCSField64(vmcs_field_encoding::kGuestVmcsLinkPtrFull, 0xFFFFFFFFFFFFFFFF);

		/**
		 *  Write host fields
		 */

		// Selectors
		status = WriteVMCSField16(vmcs_field_encoding::kHostEsSelector, segment_selector.es);
		status = WriteVMCSField16(vmcs_field_encoding::kHostCsSelector, segment_selector.cs);
		status = WriteVMCSField16(vmcs_field_encoding::kHostSsSelector, segment_selector.ss);
		status = WriteVMCSField16(vmcs_field_encoding::kHostDsSelector, segment_selector.ds);
		status = WriteVMCSField16(vmcs_field_encoding::kHostFsSelector, segment_selector.fs);
		status = WriteVMCSField16(vmcs_field_encoding::kHostGsSelector, segment_selector.gs);
		status = WriteVMCSField16(vmcs_field_encoding::kHostTrSelector, segment_selector.tr);

		// Base
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostFsBase, _readfsbase_natural());
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostGsBase, _readgsbase_natural());
		// TODO: Add tr base?
		// status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostTrBase, );
		
		// MSRs
		status = WriteVMCSField32(vmcs_field_encoding::kHostIa32SysenterCs, static_cast<UINT32>(sysenter_cs_msr));
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostIa32SysenterEsp, sysenter_esp_msr);
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostIa32SysenterEip, sysenter_eip_msr);
		
		// Registers
#ifdef __64BIT__
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostRsp, __read_rsp());
#else
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostRsp, __read_esp());
#endif
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostRip, reinterpret_cast<processor::natural_width>(_ReturnAddress()));

		// Control Registers
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostCr0, __readcr0());
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostCr3, __readcr3());
		status = WriteVMCSFieldNatural(vmcs_field_encoding::kHostCr4, __readcr4());

		/**
		 *  Control fields
		 */

		 /**
		  *  Primary Control fields
		  */

		status = WriteVMCSField32(vmcs_field_encoding::kControlPinBasedVmExecutionControls, vm_pin_exec_ctrls.all);

		vm_exec_ctrl_fields.fields.secondControls = 1;
		status = WriteVMCSField32(vmcs_field_encoding::kControlPrimaryProcessorVmExecutionControls, vm_exec_ctrl_fields.all);

		vm_secondary_ctrl_fields.fields.enableEPT = 1;
		// vm_secondary_ctrl_fields.fields.enableVPID = 1;
		status = WriteVMCSField32(vmcs_field_encoding::kControlSecondaryProcessorVmExecutionControls, vm_secondary_ctrl_fields.all);

		// Use EPT
		// TODO: Implement EPTP stuff
		// TODO: Implement PML4, PDPT, PD, PT for guest physical -> host physical translation
		// TODO: Replace `vm_secondary_ctrl_fields.all` with `eptp`
		status = WriteVMCSField64(vmcs_field_encoding::kControlEPTPFull, vm_secondary_ctrl_fields.all);

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

		status = ReadVMCSField32(vmcs_field_encoding_e::kVmInstructionError, &instruction_error);

		if (!NT_SUCCESS(status)) {
			MDbgPrint("Encountered an error but couldn't read instruction error from vmcs.\n");
			goto cleanup;
		}

		MDbgPrint("Encountered an error: %s (status code %ld).\n", 
			InstructionErrorToString(static_cast<UINT32>(instruction_error)), 
			instruction_error);

	cleanup:
		return status;
	}
	
	NTSTATUS ClearActiveVMCS() {
		NTSTATUS status = STATUS_SUCCESS;
		UINT64 vmcs_physical_addr = NULL;
		unsigned int operation_status = 0;

		operation_status = __vmx_vmptrld(&vmcs_physical_addr);
		if (operation_status != 0) {
			if (operation_status == 1) {
				PrintVMXError();
			}
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