#ifndef __MSR_H
#define __MSR_H

#include <intrin.h>
#include <wdm.h>

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
		kIa32VmxMisc				= 0x485,		// IA32_VMX_MISC
		kIa32VmxEntryCtls			= 0x484,		// IA32_VMX_ENTRY_CTLS
		kIa32VmxTrueEntryCtls		= 0x490,		// IA32_VMX_TRUE_ENTRY_CTLS
		kIa32VmxExitCtls			= 0x483,		// IA32_VMX_EXIT_CTLS
		kIa32VmxTrueExitCtls		= 0x48F,		// IA32_VMX_TRUE_EXIT_CTLS
		kIa32FsBase					= 0xc0000100,	// IA32_FS_BASE
		kIa32GsBase					= 0xc0000101,	// IA32_GS_BASE
	};

	// IA32_FEATURE_CONTROL_MSR
	typedef union {
		struct {
			unsigned lock : 1;                  //!< [0]
			unsigned enable_smx : 1;            //!< [1]
			unsigned enable_vmxon : 1;          //!< [2]
			unsigned reserved1 : 5;             //!< [3:7]
			unsigned enable_local_senter : 7;   //!< [8:14]
			unsigned enable_global_senter : 1;  //!< [15]
			unsigned reserved2 : 16;            //!<
			unsigned reserved3 : 32;            //!< [16:63]
		} bitfield;
		UINT64 all;
	} FeatureControlMsr;

	typedef struct _Bitfield64 {
		unsigned char bit0 : 1;
		unsigned char bit1 : 1;
		unsigned char bit2 : 1;
		unsigned char bit3 : 1;
		unsigned char bit4 : 1;
		unsigned char bit5 : 1;
		unsigned char bit6 : 1;
		unsigned char bit7 : 1;
		unsigned char bit8 : 1;
		unsigned char bit9 : 1;
		unsigned char bit10 : 1;
		unsigned char bit11 : 1;
		unsigned char bit12 : 1;
		unsigned char bit13 : 1;
		unsigned char bit14 : 1;
		unsigned char bit15 : 1;
		unsigned char bit16 : 1;
		unsigned char bit17 : 1;
		unsigned char bit18 : 1;
		unsigned char bit19 : 1;
		unsigned char bit20 : 1;
		unsigned char bit21 : 1;
		unsigned char bit22 : 1;
		unsigned char bit23 : 1;
		unsigned char bit24 : 1;
		unsigned char bit25 : 1;
		unsigned char bit26 : 1;
		unsigned char bit27 : 1;
		unsigned char bit28 : 1;
		unsigned char bit29 : 1;
		unsigned char bit30 : 1;
		unsigned char bit31 : 1;
		unsigned char bit32 : 1;
		unsigned char bit33 : 1;
		unsigned char bit34 : 1;
		unsigned char bit35 : 1;
		unsigned char bit36 : 1;
		unsigned char bit37 : 1;
		unsigned char bit38 : 1;
		unsigned char bit39 : 1;
		unsigned char bit40 : 1;
		unsigned char bit41 : 1;
		unsigned char bit42 : 1;
		unsigned char bit43 : 1;
		unsigned char bit44 : 1;
		unsigned char bit45 : 1;
		unsigned char bit46 : 1;
		unsigned char bit47 : 1;
		unsigned char bit48 : 1;
		unsigned char bit49 : 1;
		unsigned char bit50 : 1;
		unsigned char bit51 : 1;
		unsigned char bit52 : 1;
		unsigned char bit53 : 1;
		unsigned char bit54 : 1;
		unsigned char bit55 : 1;
		unsigned char bit56 : 1;
		unsigned char bit57 : 1;
		unsigned char bit58 : 1;
		unsigned char bit59 : 1;
		unsigned char bit60 : 1;
		unsigned char bit61 : 1;
		unsigned char bit62 : 1;
		unsigned char bit63 : 1;
	} Bitfield64;

	// IA32_VMX_BASIC
	typedef union {
		Bitfield64 bitfield;
		struct {
			unsigned revision_id : 32;          // [0:31]
			unsigned reserved3 : 32;            // [32:63]
		} fields;
		UINT64 all;
	} VmxBasicMsr;

	// IA32_VMX_MISC (page 4514)
	typedef union {
		struct {
			UINT64 time_bit_change : 5;       // [0:4]
			UINT64 exit_store_efer : 1;       // [5] - Stores IA32_EFER.LMA on VM exit. 
			
			// Activity states
			UINT64 hlt_support : 1;			  // [6]
			UINT64 shutdown_support : 1;	  // [7]
			UINT64 wait_for_sipi_support : 1; // [8]
			
			UINT64 reserved0 : 5; // [9:13]
			UINT64 intel_pt_enable : 1; // [14]
			UINT64 rdmsr_in_smm : 1; // [15]
			UINT64 cr3_target_values_supported : 9; // [16:24]
			UINT64 recommended_max_store_msr : 3; // [25:27]
			UINT64 smm_monitor_settable : 1; // [28]
			UINT64 software_use_vmwrite : 1; // [29]
			UINT64 allow_injection : 1; // [30]
			UINT64 reserved1 : 1; // [31]
			UINT64 mseg_revision_id : 32; // [32:63]
		} fields;
		UINT64 all;
	} VmxMiscMsr;

	unsigned long long ReadMsr(intel_e msr_num);
}

#endif __MSR_H