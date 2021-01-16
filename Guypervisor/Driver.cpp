#include "Driver.h"

#include "Device.h"
#include "print.h"
#include "virtualization.h"
#include "processor_context.h"

#define DEVICE_NAME L"\\Device\\Guypervisor"
#define DOS_DEVICE_NAME L"\\DosDevices\\Guypervisor"
#define HAS_CPUID_FLAG_MASK 1 << 21

Device* kGuypervisor = nullptr;
processor_context::processorContext* processor_context::kProcessorContext = nullptr;

extern "C" 
NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT		pDriverObject,
	_In_ PUNICODE_STRING	/*pUnicodeString*/
) {
    UNREFERENCED_PARAMETER(pDriverObject);
    // sxe ld guypervisor.sys
    // bu guypervisor!DriverEntry
    NTSTATUS status = STATUS_SUCCESS;
    bool vendor_is_intel, supports_vtx, cpuid_supported;
    
    // Check if the cpuid instruction is available
    cpuid_supported = IsCPUIdSupported(HAS_CPUID_FLAG_MASK);
    if (!cpuid_supported) {
        MDbgPrint("Failed because your processor doesn\'t support CPUID.\n");
        status = STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

    // Check if the vendor is Intel
    vendor_is_intel = virtualization::VendorIsIntel();
    if (!vendor_is_intel) {
        MDbgPrint("Failed because your vendor is not Intel.\n");
        status = STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

    // Check if CPU supports VTx
    supports_vtx = virtualization::SupportsVtxOperation();
    if (!supports_vtx) {
        MDbgPrint("Failed because your processor does not support VTx.\n");
        status = STATUS_NOT_SUPPORTED;
        goto cleanup;
    }
    
    kGuypervisor = new Device(DEVICE_NAME, DOS_DEVICE_NAME);
    status = kGuypervisor->InitDevice(pDriverObject);

    if (!NT_SUCCESS(status)) {
        MDbgPrint("Could not initiate device!\n");
        goto cleanup;
    }

    MDbgPrint("Successfully initiated device!\n");

    // Create symbolic link to DOS Device
    status = kGuypervisor->CreateSymlink();
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }
    MDbgPrint("Successfully created symlink!\n");

    // Initialize processor context
    processor_context::kProcessorContext = new processor_context::processorContext;
    status = processor_context::InitializeProcessorContext();

    if (!NT_SUCCESS(status))
    {
        MDbgPrint("Failed initializing processor context with error: %d\n", status);
        goto cleanup;
    }

    // TODO: Currently identitiy mapping ept, change this
    status = processor_context::InitializeEPT();
    if (!NT_SUCCESS(status))
    {
        MDbgPrint("Failed initializing the EPT with error: %d\n", status);
        goto cleanup;
    }

cleanup:
    if (!NT_SUCCESS(status)) {
        if (kGuypervisor != nullptr) 
        {
            delete kGuypervisor;
        }

        if (processor_context::kProcessorContext != nullptr)
        {
            processor_context::FreeProcessorContext();
        }
    }
	return status;
}

#undef DEVICE_NAME
#undef DOS_DEVICE_NAME
#undef HAS_CPUID_FLAG_MASK

