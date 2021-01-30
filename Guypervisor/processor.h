#ifndef __PROCESSOR_H
#define __PROCESSOR_H

#include <wdm.h>

constexpr UINT16 kRplMask = 3;

#ifdef _WIN64  
#define __64BIT__
#else
#define __32BIT__
#endif

extern "C" {
	inline UINT16 GetEsSelector();
	inline UINT16 GetCsSelector();
	inline UINT16 GetSsSelector();
	inline UINT16 GetDsSelector();
	inline UINT16 GetFsSelector();
	inline UINT16 GetGsSelector();
	inline UINT16 GetLdtrSelector();
	inline UINT16 GetTrSelector();

	// Need to define it for intrin.h
	void _sgdt(void * dest); 

	inline UINT32 __lar(UINT16 segment);

#ifdef __64BIT__
	inline UINT64 __read_rsp();
#else
	inline UINT32 __read_esp();
#endif
}

namespace processor {
	
#ifdef __64BIT__ 
	// 64 Bit
	typedef UINT64 natural_width;

#define _readfsbase_natural _readfsbase_u64
#define _readgsbase_natural _readgsbase_u64

#else
	typedef UINT32 natural_width;

#define _readfsbase_natural _readfsbase_u32
#define _readgsbase_natural _readgsbase_u32

#endif

	typedef struct _segmentSelector{
		UINT16 es;
		UINT16 cs;
		UINT16 ss;
		UINT16 ds;
		UINT16 fs;
		UINT16 gs;
		UINT16 ldtr;
		UINT16 tr;
	} segmentSelector;

#pragma pack(push,1)
	// Page 2880 
	typedef struct _gdtEntry {
		UINT16 limit0;
		UINT16 base0;
		struct {
			UINT16 base1 : 8;
			UINT16 type : 4;
			UINT16 system : 1;
			UINT16 dpl : 2;
			UINT16 present : 1;
		};
		struct {
			UINT16 limit1 : 4;
			UINT16 avl : 1;
			UINT16 l : 1;
			UINT16 d : 1;
			UINT16 g : 1;
			UINT16 base2 : 8;
		};
		UINT32 base3;
		UINT32 zeroed_out; // Must be zero
	} GDTEntry;


#pragma pack(pop)

#pragma pack(push,1)
	typedef union _AccessRights{
		struct {
			UINT16 crap;		  // [0:15]
			UINT16 unusable : 1;  // [16]
			UINT16 reserved : 15; // [17:31]
		} fields;
		UINT32 all;
	} AccessRights;
#pragma pack(pop)

#pragma pack(push,1)

	typedef union _vmxGdtEntry {
		struct {
			UINT32 limit;		// First 16 bits of the limit
			UINT64 base;	// First 16 bits of the base
			AccessRights access;
		} fields;
	} vmxGdtEntry;

#pragma pack(pop)

#pragma pack(push,1)

	typedef struct _gdtr {
		UINT16 limit;
		UINT64 base;
	} GDTR;


#pragma pack(pop)

#pragma pack(push,1)

	typedef struct _idtr {
		UINT16 limit;
		UINT64 base;
	} IDTR;

#pragma pack(pop)

	void LoadSegmentSelectors(segmentSelector* segment_selector);
	void ReadGDTEntry(GDTR gdt_base, UINT16 selector, vmxGdtEntry* entry);

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

	/*
		Control registers are documented at page 2860 all volumes.
	*/
	typedef union {
		Bitfield64 bitfield;
		struct {
			/*
			Enables protected mode when set; enables real-address mode when clear.
			This flag does not enable paging directly. It only enables segment-level protection.
			To enable paging, both the PE and PG flags must be set.
			*/
			unsigned pe : 1;			// [0] - Protection Enable
			unsigned mp : 1;			// [1] - Monitor Coprocessor
			/*
			 * Will be set if the processor doesn't support FPU instructions such as
			 * MMX/SSE/SSE2/SSE3/SSSE3/SS4
			 */
			unsigned em : 1;			// [2] - Emulation
			/*
			 * Lazy x87 task switch save MMX registers.
			 * The processor sets this flag on every task switch and tests it when
			 * executing x87 FPU/MMX/SSE/SSE2/SSE3/SSSE3/SSE4 instructions.
			 */
			unsigned ts : 1;            // [3] - Task Switched
			unsigned et : 1;			// [4] - Extension Type
			/*
			 * Enables the reporting mechanism for reporting x87 FPU errors.
			 */
			unsigned ne : 1;			// [5] - Numeric Error
			unsigned reserved1 : 10;	// [6:15] - Should be zero
			unsigned wp : 1;			// [16] - Write Protect
			unsigned reserved2 : 1;		// [17] - Should be zero
			/*
			 * Does automatic alignment checking if enabled.
			 */
			unsigned am : 1;			// [18] - Alignment Mask
			unsigned reserved3 : 10;	// [19:28] - Should be zero
			unsigned nw : 1;			// [29] - Not Write-through
			unsigned cd : 1;			// [30] - Cache Disable
			/*
			 * Enables paging when set, disables paging when clear.
			 * The PG flag has no effect if the PE flag (bit 0 of register CR0) is not also set;
			 */
			unsigned pg : 1;			// [31] - Paging
			unsigned reserved4 : 32;	// [32:63] - Should be zero
		} fields;
		UINT64 all;
	} Cr0;

	typedef union {
		Bitfield64 bitfield;
		struct {
			UINT64 reserved1 : 3;		// [0:2]
			UINT64 pwt : 1;				// [3] - Page-level Write-Through (Note: only on x64 ?)
			UINT64 pcd : 1;				// [4] - Page-level Cache Disable (Note: only on x64 ?)
			UINT64 reserved2 : 7;		// [5:11]
			UINT64 pdb : 20;			// [12:31] - Page-Directory Base
			UINT64 reserved3 : 32;		// [32:63] - Should be zero
		} fields;
		UINT64 all;
	} Cr3;

	typedef union {
		Bitfield64 bitfield;
		struct {
			unsigned vme : 1;			// [0] - Virtual-8086 Mode Extensions
			/*
			 * Enables hardware support for a virtual interrupt flag (VIF) in protected 
			 * mode when set; disables the VIF flag in protected mode when clear.
			 */
			unsigned pvi : 1;			// [1] - Protected-Mode Virtual Interrupts 
			unsigned tsd : 1;			// [2] - Time Stamp Disable 
			/*
			 * References to debug registers DR4 and DR5 cause an undefined 
			 * opcode (#UD) exception to be generated when set.
			 */
			unsigned de : 1;            // [3] - Debugging Extensions
			/*
			 * Enables 4-MByte pages with 32-bit paging when set; restricts 32-bit 
			 * paging to pages of 4 KBytes when clear.
			 */
			unsigned pse : 1;			// [4] - Page Size Extensions 
			/*
			 * When set, enables paging to produce physical addresses with more than 32 bits. 
			 * When clear, restricts physical addresses to 32 bits. PAE must be 
			 * set before entering IA-32e mode.
			 */
			unsigned pae : 1;			// [5] - Physical Address Extension
			unsigned mce : 1;			// [6] - Machine-Check Enable
			unsigned pge : 1;			// [7] - Page Global Enable 
			unsigned pce : 1;			// [8] - Performance-Monitoring Counter Enable
			unsigned osfxsr : 1;		// [9] - Operating System Support for FXSAVE and FXRSTOR instructions
			unsigned osxmmexcpt : 1;	// [10] - Operating System Support for Unmasked SIMD Floating-Point Exceptions
			unsigned umip : 1;			// [11] - User-Mode Instruction Prevention
			unsigned la57 : 1;			// [12] - 57-bit linear addresses
			unsigned vmxe : 1;			// [13] - VMX-Enable Bit
			unsigned smxe : 1;			// [14] - SMX-Enable Bit
			unsigned reserved1 : 1;		// [15]
			unsigned fsgsbase : 1;		// [16] - FSGSBASE-Enable Bit
			unsigned pcide : 1;			// [17] - PCID-Enable Bit
			unsigned osxsave : 1;		// [18] - XSAVE and Processor Extended States-Enable Bit
			unsigned reserved2 : 1;		// [19]
			unsigned smep : 1;			// [20] - SMEP-Enable Bit
			unsigned smap : 1;			// [21] - SMAP-Enable Bit
			unsigned pke : 1;			// [22] - Enable protection keys for user-mode pages 
			unsigned cet: 1;			// [23] - Control-flow Enforcement Technology
			unsigned pks : 1;			// [24] - Enable protection keys for supervisor-mode pages
			unsigned reserved3 : 16;	// [25:63]
			unsigned reserved4 : 23;	// [25:63]
		} fields;
		UINT64 all;
	} Cr4;
}

#endif /* __PROCESSOR_H */