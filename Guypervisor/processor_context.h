#ifndef __PROCESSOR_CONTEXT_H
#define __PROCESSOR_CONTEXT_H

#include "vmcs.h"
#include "paging.h"

namespace processor_context {
	struct processorContext {
		VMCS* vmxon_region;
		VMCS* vmcs_region;
		PML4E* pml4_entries;
	};

	NTSTATUS InitializeProcessorContext();
	void FreeProcessorContext();
	
	extern processorContext* kProcessorContext;
}

#endif /* __PROCESSOR_CONTEXT_H */