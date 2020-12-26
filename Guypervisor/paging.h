#include <wdm.h>

#ifndef __PAGING_H
#define __PAGING_H

constexpr unsigned int kPml4EntryCount = 512;
constexpr unsigned int kPageDirectoryPtrEntryCount = 512;
constexpr unsigned int kPageDirectoryEntryCount = 512;
constexpr unsigned int kPageTableEntryCount = 512;

/*
 * The paging tables are documented in page 2909
 */

typedef union _PML4Entry {
	struct {
		UINT64 read_access : 1;		 // [0] - Read Access
		UINT64 write_access : 1;	 // [1] - Write access
		/*
		 * If the “mode-based execute control for EPT” VM-execution control is 0 then this field is
		 * general execute access. Else, this field is supervisor-mode linear execute access.
		 */
		UINT64 execute_access : 1;	 // [2] - Execute Access
		UINT64 reserved1 : 5;		 // [3:7] - Must be zero
		UINT64 dirty_flag : 1;		 // [8] - Dirty flag is only enabled if the EPTP enabled it. (else it must be 0)
		UINT64 reserved2 : 1;		 // [9] - Ignored
		UINT64 user_exec_access : 1; // [10] - Just like bit 2, if the control is 1 then this is for usermode, else this must be zero.
		UINT64 reserved3 : 1;		 // [11] - Ignored

		/*
		 * Next page table address can be 40 bits because the next page table is
		 * 4KB aligned which coincidentally is the first 12 bits.
		 *
		 * No processors supporting the Intel 64 architecture support more than 48 physical-address bits.
		 * Thus, no such processor can produce a guest-physical address with more than 48 bits.
		 *
		 * An attempt to use such an address causes a page fault. An attempt to load CR3 with such an
		 * address causes a general-protection fault.
		 *
		 * If PAE paging is being used, an attempt to load CR3 that would load a PDPTE with such an
		 * address causes a general-protection fault.
		*/
		UINT64 next_page_table : 40; // [12:51] - From the CR3
		UINT64 reserved4 : 12; // [52:63] - Ignored
	} fields;
	UINT64 all;
} PML4E;

typedef union _PDPTEntry {
	struct {
		UINT64 read_access : 1;		 // [0] - Read Access
		UINT64 write_access : 1;	 // [1] - Write access
		/*
			* If the “mode-based execute control for EPT” VM-execution control is 0 then this field is
			* general execute access. Else, this field is supervisor-mode linear execute access.
			*/
		UINT64 execute_access : 1;	 // [2] - Execute Access
		UINT64 reserved1 : 5;		 // [3:7] - Must be zero
		UINT64 dirty_flag : 1;		 // [8] - Dirty flag is only enabled if the EPTP enabled it. (else it must be 0)
		UINT64 reserved2 : 1;		 // [9] - Ignored
		UINT64 user_exec_access : 1; // [10] - Just like bit 2, if the control is 1 then this is for usermode, else this must be zero.
		UINT64 reserved3 : 1;		 // [11] - Ignored

		/*
			* Next page table address can be 40 bits because the next page table is
			* 4KB aligned which coincidentally is the first 12 bits.
			*
			* No processors supporting the Intel 64 architecture support more than 48 physical-address bits.
			* Thus, no such processor can produce a guest-physical address with more than 48 bits.
			*
			* An attempt to use such an address causes a page fault. An attempt to load CR3 with such an
			* address causes a general-protection fault.
			*
			* If PAE paging is being used, an attempt to load CR3 that would load a PDPTE with such an
			* address causes a general-protection fault.
		*/
		UINT64 next_page_table : 40; // [12:51] - From the CR3
		UINT64 reserved4 : 12; // [52:63] - Ignored
	} fields;
	UINT64 all;
} PDPTE;

typedef union _PDEntry {
	struct {
		UINT64 read_access : 1;		 // [0] - Read Access
		UINT64 write_access : 1;	 // [1] - Write access
		/*
			* If the “mode-based execute control for EPT” VM-execution control is 0 then this field is
			* general execute access. Else, this field is supervisor-mode linear execute access.
		*/
		UINT64 execute_access : 1;	 // [2] - Execute Access
		UINT64 reserved1 : 5;		 // [3:7] - Must be zero
		UINT64 dirty_flag : 1;		 // [8] - Dirty flag is only enabled if the EPTP enabled it. (else it must be 0)
		UINT64 reserved2 : 1;		 // [9] - Ignored
		UINT64 user_exec_access : 1; // [10] - Just like bit 2, if the control is 1 then this is for usermode, else this must be zero.
		UINT64 reserved3 : 1;		 // [11] - Ignored

		/*
			* Next page table address can be 40 bits because the next page table is
			* 4KB aligned which coincidentally is the first 12 bits.
			*
			* No processors supporting the Intel 64 architecture support more than 48 physical-address bits.
			* Thus, no such processor can produce a guest-physical address with more than 48 bits.
			*
			* An attempt to use such an address causes a page fault. An attempt to load CR3 with such an
			* address causes a general-protection fault.
			*
			* If PAE paging is being used, an attempt to load CR3 that would load a PDPTE with such an
			* address causes a general-protection fault.
		*/
		UINT64 next_page_table : 40; // [12:51] - From the CR3
		UINT64 reserved4 : 12; // [52:63] - Ignored
	} fields;
	UINT64 all;
} PDE;

typedef union _PTEntry {
	struct {
		UINT64 read_access : 1;		 // [0] - Read Access
		UINT64 write_access : 1;	 // [1] - Write access
		/*
		 * If the “mode-based execute control for EPT” VM-execution control is 0 then this field is
		 * general execute access. Else, this field is supervisor-mode linear execute access.
		 */
		UINT64 execute_access : 1;	   // [2] - Execute Access
		UINT64 memory_type : 3;		   // [3:5] - EPT Memory type
		UINT64 ignore_memory_type : 1; // [6] - Ignore the previous bit
		UINT64 reserved1 : 1;		   // [7] - Ignored
		UINT64 dirty_flag : 1;		   // [8] - Dirty flag is only enabled if the EPTP enabled it. (else it must be 0)
		UINT64 reserved2 : 1;		   // [9] - Ignored
		UINT64 user_exec_access : 1;   // [10] - Just like bit 2, if the control is 1 then this is for usermode, else this must be zero.
		UINT64 reserved3 : 1;		   // [11] - Ignored

		/*
		 * This contains the final address for the entry
		 */
		UINT64 final_address : 40;	   // [12:51] - Final physical address
		UINT64 reserved4 : 11;		   // [52:62] - Ignored
		/*
		 * If “EPT-violation #VE” VMexecution control is 0, this bit is ignored.
		 */
		UINT64 supress_ve : 1;		   // [63] - Supress #VE violations.
	} fields;
	UINT64 all;
} PTE;

#endif /* __PAGING_H */