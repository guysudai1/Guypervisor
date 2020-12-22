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
		processor::natural_width test_if_worked = 0;
		
		// Write test
		//status = WriteVMCSField64(vmcs_field_encoding::kGuestRip, 0x0000133713371337);
		//if (!NT_SUCCESS(status)) {
		//	MDbgPrint("FAILED VMCS WRITE :(\n");
		//	goto cleanup;
		//}

		//ReadVMCSFieldNatural(vmcs_field_encoding::kGuestRip, &test_if_worked);
	cleanup:
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