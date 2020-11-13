#pragma once

#include "vmcs.h"

struct processorContext {
	VMCS* vmxon_region;
	VMCS* vmcs_region;
};

namespace Processor {
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
	} CPUInfo, * pCPUInfo;

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
	} ControlRegister_t, * pControlRegister_t;
}