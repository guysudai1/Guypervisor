#include "processor_context.h"

#undef _NTDDK_
#include <ntddk.h>

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

	kProcessorContext->ept_pml4_entries = new PML4E[kPml4EntryCount];
	RtlZeroMemory(kProcessorContext->ept_pml4_entries, kPml4EntryCount * sizeof(PML4E));
	
	kProcessorContext->msr_bitmap = static_cast<UINT8*>(AllocateContingiousPhysicalMemoryAligned(
		sizeof(UINT8), kAlignmentSize
	));

	kProcessorContext->guest_stack = new UINT8[kStackSize];
	kProcessorContext->host_stack = new UINT8[kStackSize];
cleanup:
	return status;
}

NTSTATUS processor_context::InitializeEPT()
{
	NTSTATUS status = STATUS_SUCCESS;

	auto pml4_entries = processor_context::kProcessorContext->ept_pml4_entries;
	auto pdpte_entries = new PDPTE[kPdptEntryCount];
	RtlZeroMemory(pdpte_entries, kPdptEntryCount);

	auto pde_entries = new PDPTE[kPdptEntryCount][kPageDirectoryEntryCount];
	RtlZeroMemory(pde_entries, kPageDirectoryEntryCount * kPdptEntryCount);

	pml4_entries[0].fields.read_access = 1;
	pml4_entries[0].fields.write_access = 1;
	pml4_entries[0].fields.execute_access = 1;
	pml4_entries[0].fields.next_page_table = MmGetPhysicalAddress(pdpte_entries).QuadPart / PAGE_SIZE;

	for (size_t i = 0; i < kPdptEntryCount; ++i) {
		auto temp = pdpte_entries[i];
		temp.fields.read_access = temp.fields.write_access = temp.fields.execute_access = 1;
		temp.fields.next_page_table = MmGetPhysicalAddress(&pde_entries[i][0]).QuadPart / PAGE_SIZE;
	}

	for (size_t i = 0; i < kPdptEntryCount; ++i) {
		/*
		 * Maps 2MB per PDPTE
		 */
		for (size_t j = 0; j < kPageDirectoryEntryCount; ++j) {
			auto temp = pde_entries[i][j];
			temp.fields.read_access = temp.fields.write_access = temp.fields.execute_access = 1;
			temp.fields.next_page_table = MmGetPhysicalAddress(&pde_entries[i][0]).QuadPart / PAGE_SIZE;
		}
	}

	return status;

}

void processor_context::FreeProcessorContext()
{
	IF_NOT_NULLPTR_THEN_FREE_AND_SET_NULLPTR(kProcessorContext->vmxon_region);
	IF_NOT_NULLPTR_THEN_FREE_AND_SET_NULLPTR(kProcessorContext->vmcs_region);
	IF_NOT_NULLPTR_THEN_FREE_AND_SET_NULLPTR(kProcessorContext->msr_bitmap);

	if (kProcessorContext->guest_stack != nullptr) {
		kProcessorContext->guest_stack = nullptr;
		delete kProcessorContext->guest_stack;
	}

	if (kProcessorContext->host_stack != nullptr) {
		kProcessorContext->host_stack = nullptr;
		delete kProcessorContext->host_stack;
	}

	if (kProcessorContext->ept_pml4_entries != nullptr) {
		kProcessorContext->ept_pml4_entries = nullptr;
		delete kProcessorContext->ept_pml4_entries;
	}

	//if (kProcessorContext->io_bitmap_a != nullptr) {
	//	kProcessorContext->io_bitmap_a = nullptr;
	//	delete kProcessorContext->io_bitmap_a;
	//}

	//if (kProcessorContext->io_bitmap_b != nullptr) {
	//	kProcessorContext->io_bitmap_b = nullptr;
	//	delete kProcessorContext->io_bitmap_b;
	//}

	if (kProcessorContext != nullptr)
	{
		kProcessorContext = nullptr;
		delete kProcessorContext;
	}
}
