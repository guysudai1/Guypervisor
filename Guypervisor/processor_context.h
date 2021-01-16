#ifndef __PROCESSOR_CONTEXT_H
#define __PROCESSOR_CONTEXT_H

#include "vmcs.h"
#include "paging.h"

constexpr uintptr_t kAlignmentSize = 4 * 1024;// 4KB
constexpr size_t kStackSize = 12 * PAGE_SIZE;

namespace processor_context {
	struct processorContext {
		VMCS* vmxon_region;
		VMCS* vmcs_region;
		PML4E* ept_pml4_entries;
		//UINT8* io_bitmap_a;
		//UINT8* io_bitmap_b;

		UINT8* msr_bitmap;
		UINT8* guest_stack;
		UINT8* host_stack;
	};

	NTSTATUS InitializeProcessorContext();
	NTSTATUS InitializeEPT();
	void FreeProcessorContext();
	
	extern processorContext* kProcessorContext;
}

#endif /* __PROCESSOR_CONTEXT_H */