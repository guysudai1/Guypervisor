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


namespace virtualization {
	NTSTATUS EnterVmxonMode()
	{
		// Set CR4.VMXE [bit 13] = 1
		// readmsr 0x3A and modify with:
		//  - Lock bit [0 bit] = 1
		//  - SMX Operation [1 bit] = 1
		//  - SMX out of operation [2 bit] = 1
		NTSTATUS status = STATUS_SUCCESS;

		processor::ControlRegister cr0{0};
		processor::Cr4 cr4{0};
		processor::FeatureControlMsr controlMsr{0};
		processor::VmxBasicMsr basicMsr{0};
		ULONG64 physicalAllocatedVMX;
		unsigned char operationStatus = 0;
		processor::processorContext* context = processor::kProcessorContext;

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
		processor::processorContext* context = processor::kProcessorContext;
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

	NTSTATUS LaunchGuest()
	{
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

	NTSTATUS EncodeVMArgument(AccessType access_bit, 
							  UINT8 index, 
							  ComponentType component_type, 
							  WidthType width_type, 
							  FieldEncoding** encoding) {
		// TODO: Possibly fix bug here, don't think it's outputting correctly
		NTSTATUS status = STATUS_SUCCESS;

		if (encoding == nullptr) {
			status = STATUS_NULLPTR_ERROR;
			goto cleanup;
		}
		FieldEncoding* encoding_field = new FieldEncoding{};
		encoding_field->fields.access_type = access_bit;
		encoding_field->fields.index = index;
		encoding_field->fields.field_width = width_type;
		encoding_field->fields.component_type = component_type;

		*encoding = encoding_field;
	cleanup:
		return status;
	}

	NTSTATUS PrintVMXError() {
		NTSTATUS status = STATUS_SUCCESS;
		SIZE_T fld = 0;
		// TODO: Add read information
		status = ReadVMCSField(0, ComponentType::kExitInfoComponent, WidthType::k32Bit, &fld);

		if (!NT_SUCCESS(status)) {
			goto cleanup;
		}

	cleanup:
		return status;
	}

	NTSTATUS ReadVMCSField(UINT8 index,
						   ComponentType component_type,
						   WidthType width_type,
						   SIZE_T* field) {
		NTSTATUS status = STATUS_SUCCESS;
		FieldEncoding* encoding = nullptr;
		AccessType access_bit = (width_type == WidthType::k64Bit) ? 
									AccessType::kHighAccess : AccessType::kFullAccess;
		unsigned char operationStatus = 0;
		size_t test = 0;

		if (field == nullptr) {
			status = STATUS_NULLPTR_ERROR;
			goto cleanup;
		}

		status = EncodeVMArgument(access_bit, index, component_type, width_type, &encoding);

		if (!NT_SUCCESS(status)) {
			goto cleanup;
		}
		// encoding->all = 0x00004402; 
		// TODO: Figure out why vmread crashes windows.
		operationStatus = __vmx_vmread(static_cast<size_t>(encoding->all), &test);
		if (operationStatus != 0) {
			status = STATUS_FAILED_VMREAD;
			// TODO: Try to read the error information section
			goto cleanup;
		}

	cleanup:
		if (encoding != nullptr) {
			// Free
			delete encoding;
		}
		return status;
	}
}