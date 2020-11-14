#ifndef __VIRTUALIZATION_H
#define __VIRTUALIZATION_H

extern "C" bool is_cpuid_supported(unsigned int mask);

namespace virtualization {
	/*
	 * Contains IOCTL numbers needed to interact with device
	 */
	enum IoctlCodes {
		ENTER_VMX = 0x801
	};

	bool vendor_is_intel();
	bool supports_vtx_operation();

	bool enter_vmxon_mode();
}

#endif /* __VIRTUALIZATION_H */