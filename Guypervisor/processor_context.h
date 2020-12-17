#ifndef __PROCESSOR_CONTEXT_H
#define __PROCESSOR_CONTEXT_H

#include "vmcs.h"

namespace processor_context {
	struct processorContext {
		VMCS* vmxon_region;
		VMCS* vmcs_region;
		// TODO: Add EPT pointer here
	};

	NTSTATUS InitializeProcessorContext();
	void FreeProcessorContext();
	
	extern processorContext* kProcessorContext;
}

#endif /* __PROCESSOR_CONTEXT_H */