#include "Driver.h"
#include "Device.h"
#include "print.h"
#include "virtualization_detection.h"

#define DEVICE_NAME L"\\Device\\Guypervisor"
#define DOS_DEVICE_NAME L"\\DosDevices\\Guypervisor"

Device* guyPervisor;

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
    cpuid_supported = is_cpuid_supported(HAS_CPUID_FLAG_MASK);
    if (!cpuid_supported) {
        MDbgPrint("Failed because your processor doesn\'t support CPUID.");
        status = STATUS_NOT_SUPPORTED;
        return status;
    }

    // Check if the vendor is Intel
    vendor_is_intel = virtualization::vendor_is_intel();
    if (!vendor_is_intel) {
        MDbgPrint("Failed because your vendor is not Intel.");
        status = STATUS_NOT_SUPPORTED;
        return status;
    }

    // Check if CPU supports VTx
    supports_vtx = virtualization::supports_vtx_operation();
    if (!supports_vtx) {
        MDbgPrint("Failed because your processor does not support VTx.");
        status = STATUS_NOT_SUPPORTED;
        return status;
    }
    

    guyPervisor =  new Device(DEVICE_NAME, DOS_DEVICE_NAME);
    status = guyPervisor->InitDevice(pDriverObject);

    if (!NT_SUCCESS(status)) {
        MDbgPrint("Could not initiate device!");
        delete guyPervisor;
        return status;
    }

    MDbgPrint("Successfully initiated device!");
    // Initialize WDF.

    // Create symbolic link to DOS Device
    status = guyPervisor->CreateSymlink();
    if (!NT_SUCCESS(status)) {
        delete guyPervisor;
        return status;
    }
    MDbgPrint("Successfully created symlink!");

	return status;
}


void* operator new(size_t count)
{
    // Allocate new device object
    return ExAllocatePoolWithTag(NonPagedPoolExecute,
        count,
        'cool');
}

void operator delete (void* p) {
    // Free device object
    ExFreePoolWithTag(p,
        'cool');
}

void operator delete (void* p, unsigned __int64 size) {
    UNREFERENCED_PARAMETER(size);
    // Free device object
    ExFreePoolWithTag(p,
        'cool');
}