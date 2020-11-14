#ifndef __PROCESSOR_CONTEXT_H
#define __PROCESSOR_CONTEXT_H

#include "vmcs.h"

struct processorContext {
	VMCS* vmxon_region;
	VMCS* vmcs_region;
};

namespace processor {
	typedef union {
		struct {
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
		} bitfield;
		struct {
			unsigned char bit31 : 1;
			unsigned char bit30 : 1;
			unsigned char bit29 : 1;
			unsigned char bit28 : 1;
			unsigned char bit27 : 1;
			unsigned char bit26 : 1;
			unsigned char bit25 : 1;
			unsigned char bit24 : 1;
			unsigned char bit23 : 1;
			unsigned char bit22 : 1;
			unsigned char bit21 : 1;
			unsigned char bit20 : 1;
			unsigned char bit19 : 1;
			unsigned char bit18 : 1;
			unsigned char bit17 : 1;
			unsigned char bit16 : 1;
			unsigned char bit15 : 1;
			unsigned char bit14 : 1;
			unsigned char bit13 : 1;
			unsigned char bit12 : 1;
			unsigned char bit11 : 1;
			unsigned char bit10 : 1;
			unsigned char bit9 : 1;
			unsigned char bit8 : 1;
			unsigned char bit7 : 1;
			unsigned char bit6 : 1;
			unsigned char bit5 : 1;
			unsigned char bit4 : 1;
			unsigned char bit3 : 1;
			unsigned char bit2 : 1;
			unsigned char bit1 : 1;
			unsigned char bit0 : 1;
		} rev_bitfield;
		UINT32 all;
	} Register32;

	// Defines a struct for usage with CPUInfo
	typedef union {
		struct {
			Register32 eax;
			Register32 ebx;
			Register32 ecx;
			Register32 edx;
		} regs;
	} CPUInfo, * pCPUInfo;

	typedef union {
		struct {
			unsigned int _;		// eax
			char firstPart[4];	// ebx
			char thirdPart[4];	// ecx
			char secondPart[4];	// edx
		} vendorID;
	} VendorCPUInfo;

	// Defines a struct for usage with the control registers
	typedef union {
		struct
		{
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
		} bitfield;
		UINT64 all;
	} ControlRegister, * pControlRegister;

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

	// IA32_VMX_BASIC
	typedef union {
		struct {
			unsigned revision_id : 32;          // [0:31]
			unsigned reserved3 : 32;            // [32:63]
		} bitfield;
		UINT64 all;
	} VmxBasicMsr;

	// CR4
	typedef union {
		struct {
			unsigned vme : 1;			// [0]
			unsigned pvi : 1;			// [1]
			unsigned tsd : 1;			// [2]
			unsigned de  : 1;           // [3]
			unsigned pse : 1;			// [4]
			unsigned pae : 1;			// [5]
			unsigned mce : 1;			// [6]
			unsigned pge : 1;			// [7]
			unsigned pce : 1;			// [8]
			unsigned osfxsr : 1;		// [9]
			unsigned osxmmexcpt : 1;	// [10]
			unsigned umip : 1;			// [11]
			unsigned reserved1 : 1;		// [12]
			unsigned vmxe : 1;			// [13]
			unsigned smxe : 1;			// [14]
			unsigned reserved2 : 2;		// [15:16]
			unsigned pcide : 1;			// [17]
			unsigned osxsave : 1;		// [18]
			unsigned reserved3 : 1;		// [19]
			unsigned smep : 1;			// [20]
			unsigned smap : 1;			// [21]
			unsigned reserved4 : 16;	// [22:63]
			unsigned reserved5 : 26;	// [22:63]
		} bitfield;
		UINT64 all;
	} Cr4;
}

#endif /* __PROCESSOR_CONTEXT_H */