#ifndef __VIRTUALIZATION_H
#define __VIRTUALIZATION_H

#include <wdm.h>

#include "vmcs.h"
#include "vmcs_field_index.h"
#include "msr.h"

extern "C" bool IsCPUIdSupported(unsigned int mask);

namespace virtualization {
	/*
	 * Contains IOCTL numbers needed to interact with device
	 */
	enum IoctlCodes {
		kEnterVmx = 0x801
	};

	bool VendorIsIntel();
	bool SupportsVtxOperation();

	/*
	 * VT-x functions (VMXON / VMPTRLD / VMLAUNCH)
	 */
	NTSTATUS EnterVmxonMode();
	NTSTATUS InitializeVMCS();
	NTSTATUS PopulateActiveVMCS();
	NTSTATUS LaunchGuest();

	NTSTATUS PrintVMXError();
	
	NTSTATUS ClearActiveVMCS();

	/*
	 * This function is used to consult with control MSRs
	 */
	UINT32 ModifyControlValue(msr::intel_e msr, UINT32 requested_value);

	/**
	 * VMX VMWrite functions (64 bit, 32 bit, natural width)
	 */

	NTSTATUS WriteVMCSFieldNatural(
		vmcs_field_encoding_e encoding,
		processor::natural_width value
	);

	NTSTATUS WriteVMCSField64(
		vmcs_field_encoding_e encoding,
		UINT64 value
	);

	NTSTATUS WriteVMCSField32(
		vmcs_field_encoding_e encoding,
		UINT32 value
	);

	NTSTATUS WriteVMCSField16(
		vmcs_field_encoding_e encoding,
		UINT16 value
	);

	/**
	 * VMX VMRead functions (64 bit, 32 bit, natural width)
	 */

	NTSTATUS ReadVMCSField64(
		vmcs_field_encoding_e encoding,
		UINT64* field
	);

	template <class T>
	NTSTATUS ReadVMCSFieldGeneric(
		vmcs_field_encoding_e encoding,
		T* field
	);

	void DumpVmcsGuestArea();

	NTSTATUS ReadVMCSFieldNatural(
		vmcs_field_encoding_e encoding,
		processor::natural_width* field
	);


	/**
	 * VMX VMWrite functions (64 bit, 32 bit, natural width)
	 */

	NTSTATUS WriteVMCSFieldNatural(vmcs_field_encoding_e encoding,
								   processor::natural_width field);
}

#endif /* __VIRTUALIZATION_H */