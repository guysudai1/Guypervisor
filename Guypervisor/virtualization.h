#ifndef __VIRTUALIZATION_H
#define __VIRTUALIZATION_H

#include <wdm.h>

#include "vmcs.h"

extern "C" bool IsCPUIdSupported(unsigned int mask);

namespace virtualization {
	/*
	 * Contains IOCTL numbers needed to interact with device
	 */
	enum IoctlCodes {
		kEnterVmx = 0x801
	};

	/*
	 * Contains size of field
	 */
	enum AccessType {
		kFullAccess = 0x0, // 16-bit / 32-bit / natural-width bit field
		kHighAccess = 0x1  // 64-bit field access
	};

	/*
	 * Contains which component the field was in
	 */
	enum ComponentType {
		kControlComponent = 0x0,	// Control
		kExitInfoComponent = 0x1,	// VM-exit information
		kGuestStateComponent = 0x2, // Guest State
		kHostStateComponent = 0x3, // Host State
	};
	
	/*
	 * Field width
	 */
	enum WidthType {
		k16Bit = 0x0, // 16 Bit field width 
		k64Bit = 0x1, // 64 Bit field width
		k32Bit = 0x2, // Guest State
		kNaturalWidth = 0x3, // Host State
	};

	bool VendorIsIntel();
	bool SupportsVtxOperation();

	/*
	 * VT-x functions (VMXON / VMPTRLD / VMLAUNCH)
	 */
	NTSTATUS EnterVmxonMode();
	NTSTATUS InitializeVMCS();
	NTSTATUS LaunchGuest();

	NTSTATUS PrintVMXError();

	NTSTATUS EncodeVMArgument(AccessType access_bit,
							  UINT8 index,
							  ComponentType component_type,
							  WidthType width_type, 
							  FieldEncoding** encoding);
	
	NTSTATUS ReadVMCSField(UINT8 index,
						   ComponentType component_type,
						   WidthType width_type,
						   SIZE_T* field);
}

#endif /* __VIRTUALIZATION_H */