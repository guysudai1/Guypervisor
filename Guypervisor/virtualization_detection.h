#pragma once

#define HAS_CPUID_FLAG_MASK 1 << 21

extern "C" bool is_cpuid_supported(unsigned int mask);

namespace virtualization {


	typedef union {
		struct {
			unsigned int eax;
			unsigned int ebx;
			unsigned int ecx;
			unsigned int edx;
		} regs;
		struct {
			unsigned int _;		// eax
			char firstPart[4];	// ebx
			char thirdPart[4];	// ecx
			char secondPart[4];	// edx
		} vendorID;
	} cpuinfo, *pcpuinfo;

	bool vendor_is_intel();
	bool supports_vtx_operation();

	void enter_vmxon_mode();
}