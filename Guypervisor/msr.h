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
		kIa32DebugCtl				= 0x01D9,		// IA32_DEBUGCTL
		kIa32SysenterCs				= 0x174,		// IA32_SYSENTER_CS
		kIa32SysenterEsp			= 0x175,		// IA32_SYSENTER_ESP
		kIa32SysenterEip			= 0x176,		// IA32_SYSENTER_EIP
		kIa32VmxPinbasedCtls		= 0x481,		// IA32_VMX_PINBASED_CTLS
		kIa32VmxTruePinbasedCtls	= 0x48D,		// IA32_VMX_TRUE_PINBASED_CTLS
		kIa32VmxProcbasedCtls		= 0x482,		// IA32_VMX_PROCBASED_CTLS 
		kIa32VmxTrueProcbasedCtls	= 0x48E,		// IA32_VMX_TRUE_PROCBASED_CTLS
		kIa32VmxProcbasedCtls2		= 0x48B,		// IA32_VMX_PROCBASED_CTLS2
	};
}

#endif __MSR_H