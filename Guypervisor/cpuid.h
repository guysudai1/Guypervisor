#ifndef __CPUID_H
#define __CPUID_H

namespace cpuid {
	enum intel_e : unsigned long {
		kIa32CpuVendorName			= 0x0,
		kIa32CpuVersion				= 0x1,
		kIa32AddressWidth			= 0x80000008	// ADDRESS_WIDTH_CPUID 
	};
}

#endif /* __CPUID_H */
