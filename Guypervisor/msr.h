#ifndef __MSR_H
#define __MSR_H

namespace msr {
	enum class intel_e : unsigned long {
		kIa32FeatureControl			= 0x3A,			// IA32_FEATURE_CONTROL_MSR
		kIa32VmxBasic				= 0x480,		// IA32_VMX_BASIC
		kIa32VmxCr0Fixed0			= 0x486,		// IA32_VMX_CR0_FIXED0
		kIa32VmxCr0Fixed1			= 0x487,		// IA32_VMX_CR0_FIXED1
		kIa32VmxCr4Fixed0			= 0x488,		// IA32_VMX_CR4_FIXED0
		kIa32VmxCr4Fixed1			= 0x489,		// IA32_VMX_CR4_FIXED1
	};
}

#endif __MSR_H