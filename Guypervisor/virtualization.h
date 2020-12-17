#ifndef __VIRTUALIZATION_H
#define __VIRTUALIZATION_H

#include <wdm.h>

#include "vmcs.h"
#include "vmcs_field_index.h"

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
	
	/**
	 * VMX VMRead functions (64 bit, 32 bit, natural width)
	 */

	NTSTATUS ReadVMCSField64(
		vmcs_field_encoding_e encoding,
		UINT64* field
	);

	NTSTATUS ReadVMCSField32(
		vmcs_field_encoding_e encoding,
		UINT32* field
	);

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