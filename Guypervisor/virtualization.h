#ifndef __VIRTUALIZATION_H
#define __VIRTUALIZATION_H

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

	bool EnterVmxonMode();
}

#endif /* __VIRTUALIZATION_H */