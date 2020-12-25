#include "virtual_addr_helpers.h"

#include <intrin.h>

#include "processor_context.h"
#include "cpuid.h"
#include "print.h"

void AcquireMaxPhysicalAddress(PHYSICAL_ADDRESS& maxPhys)
{
	/*
	2. Software can determine a processor’s physical - address width by executing CPUID with 80000008H in EAX.The physical - address
		width is returned in bits 7:0 of EAX.
	*/
	PHYSICAL_ADDRESS MAX_PHYS = { 0 };
	processor::CPUInfo currentCpu;
	unsigned char phys_width;

	__cpuid(reinterpret_cast<int*>(&currentCpu), cpuid::kIa32AddressWidth);

	// Acquire bits 7:0
	phys_width = currentCpu.regs.eax.all & 0xff; 
	MAX_PHYS.QuadPart = static_cast<LONGLONG>(1) << static_cast<LONGLONG>(phys_width);

	maxPhys = MAX_PHYS;
}

PVOID AllocateContingiousPhysicalMemoryAligned(SIZE_T size, SIZE_T alignmentSize)
{
	PHYSICAL_ADDRESS maxPhysical = { 0 };
	PVOID allocatedMemory;

	// Acquire max physical address
	AcquireMaxPhysicalAddress(maxPhysical);

	// Allocate 4KB aligned memory (not aligned yet)
	allocatedMemory = MmAllocateContiguousMemory(size + alignmentSize, maxPhysical);

	if (!allocatedMemory) {
		MDbgPrint("MmAllocateContiguousMemory cannot allocate %ud bytes with alignment of %ud \n",
				  size, alignmentSize);
		return nullptr;
	}

	RtlZeroMemory(allocatedMemory, size + alignmentSize);

	// Align to 4KB
	allocatedMemory = reinterpret_cast<PVOID>(
		(reinterpret_cast<uintptr_t>(allocatedMemory) + alignmentSize - 1) & ~(alignmentSize - 1)
	);

	return allocatedMemory;
}


BOOLEAN WritePhysicalMemory(HANDLE PhysicalMemory, PVOID Address, SIZE_T Length)
{
    NTSTATUS            ntStatus;
    PHYSICAL_ADDRESS    viewBase{};
    PUCHAR              mappedBuffer = NULL;
	char				test[] = { 0x13, 0x37, 0x13, 0x37 };
    viewBase.QuadPart = reinterpret_cast<ULONGLONG>(Address);

	if (Length < 4) {
		return STATUS_FAILED_TO_ALLOCATE_MEMORY;
	}

    ntStatus = ZwMapViewOfSection(
		PhysicalMemory, 
		reinterpret_cast<HANDLE>(-1),                          
		reinterpret_cast<PVOID*>(&mappedBuffer), 
		0L,
		Length, 
		&viewBase, 
		&Length, 
		ViewUnmap, 
		0,
		PAGE_READWRITE
    );

    if (!NT_SUCCESS(ntStatus)) {
        MDbgPrint("Could not map view of %X length %X : %lx\n", Address, Length, ntStatus);
        return FALSE;
    }

	RtlCopyMemory(mappedBuffer, test, sizeof(test));
	ZwUnmapViewOfSection((HANDLE)-1, mappedBuffer);
	MDbgPrint("Write: %x %x %x %x\n", test[0], test[1], test[2], test[3]);
	return TRUE;
}

BOOLEAN ReadPhysicalMemory(HANDLE PhysicalMemory, PVOID Address, SIZE_T Length)
{
	NTSTATUS            ntStatus;
	PHYSICAL_ADDRESS    viewBase{};
	PUCHAR              mappedBuffer = NULL;
	char				test[] = { 0x00, 0x00, 0x00, 0x00 };
	viewBase.QuadPart = reinterpret_cast<ULONGLONG>(Address);

	if (Length < 4) {
		return STATUS_FAILED_TO_ALLOCATE_MEMORY;
	}

	ntStatus = ZwMapViewOfSection(
		PhysicalMemory,
		reinterpret_cast<HANDLE>(-1),
		reinterpret_cast<PVOID*>(&mappedBuffer),
		0L,
		Length,
		&viewBase,
		&Length,
		ViewUnmap,
		0,
		PAGE_READWRITE
	);

	if (!NT_SUCCESS(ntStatus)) {
		MDbgPrint("Could not map view of %X length %X : %lx\n", Address, Length, ntStatus);
		return FALSE;
	}

	RtlCopyMemory(test, mappedBuffer, sizeof(test));
	ZwUnmapViewOfSection((HANDLE)-1, mappedBuffer);
	MDbgPrint("Read: %x %x %x %x\n", test[0], test[1], test[2], test[3]);
	return TRUE;
}

HANDLE OpenPhysicalMemory()
{
    NTSTATUS        status;
    HANDLE          physmem;
    UNICODE_STRING  physmemString;
    OBJECT_ATTRIBUTES attributes;

    RtlInitUnicodeString(&physmemString, L"\\Device\\PhysicalMemory");

    InitializeObjectAttributes(&attributes, &physmemString, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    status = ZwOpenSection(&physmem, GENERIC_WRITE | GENERIC_READ, &attributes);

    if (!NT_SUCCESS(status)) {
        MDbgPrint("Failed ZwOpenSection(\\Device\\PhysicalMemory) => %08X\n", status);
        return NULL;
    }

    return physmem;
}