#ifndef __VIRTUALIZATION_H
#define __VIRTUALIZATION_H

#include <wdm.h>

extern "C" bool IsCPUIdSupported(unsigned int mask);

namespace virtualization {
	/*
	 * Contains IOCTL numbers needed to interact with device
	 */
	enum IoctlCodes {
		ENTER_VMX = 0x801
	};

	bool VendorIsIntel();
	bool SupportsVtxOperation();

	/*
	 * VT-x functions (VMXON / VMPTRLD / VMLAUNCH)
	 */
	NTSTATUS EnterVmxonMode();
	NTSTATUS InitializeVMCS();
	NTSTATUS LaunchGuest();
}

#endif /* __VIRTUALIZATION_H */