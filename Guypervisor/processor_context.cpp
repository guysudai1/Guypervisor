#include "processor_context.h"

#include "virtual_addr_helpers.h"
#include "print.h"
#include "cleanup.h"

#define IF_NOT_NULLPTR_THEN_FREE_AND_SET_NULLPTR(VAR) \
  do { \
	if (VAR != nullptr) { \
		MmFreeContiguousMemory(VAR); \
		VAR = nullptr; \
	} \
  } while (0)

constexpr uintptr_t kAlignmentSize = 4 * 1024;// 4KB

NTSTATUS processor_context::InitializeProcessorContext()
{
	NTSTATUS status = STATUS_SUCCESS;

	kProcessorContext->vmxon_region = static_cast<VMCS*>(AllocateContingiousPhysicalMemoryAligned(
		sizeof(*kProcessorContext->vmxon_region),
		kAlignmentSize
	));

	if (kProcessorContext->vmxon_region == nullptr) {
		status = NT_ERROR(STATUS_FAILED_TO_ALLOCATE_MEMORY);
		goto cleanup;
	}

	kProcessorContext->vmcs_region = static_cast<VMCS*>(AllocateContingiousPhysicalMemoryAligned(
		sizeof(*kProcessorContext->vmcs_region),
		kAlignmentSize
	));

	if (kProcessorContext->vmcs_region == nullptr) {
		status = STATUS_FAILED_TO_ALLOCATE_MEMORY;
		goto cleanup;
	}

	kProcessorContext->pml4_entries = new PML4E[kPml4EntryCount];
	RtlZeroMemory(kProcessorContext->pml4_entries, kPml4EntryCount);

cleanup:
	return status;
}

void processor_context::FreeProcessorContext()
{
	IF_NOT_NULLPTR_THEN_FREE_AND_SET_NULLPTR(kProcessorContext->vmxon_region);
	IF_NOT_NULLPTR_THEN_FREE_AND_SET_NULLPTR(kProcessorContext->vmcs_region);

	if (kProcessorContext->pml4_entries != nullptr) {
		delete kProcessorContext->pml4_entries;
	}
	if (kProcessorContext != nullptr)
	{
		delete kProcessorContext;
	}
}
