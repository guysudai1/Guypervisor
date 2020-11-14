#include "Driver.h"

#include "Device.h"
#include "print.h"
#include "virtualization.h"

#define DEVICE_NAME L"\\Device\\Guypervisor"
#define DOS_DEVICE_NAME L"\\DosDevices\\Guypervisor"
#define HAS_CPUID_FLAG_MASK 1 << 21

Device* g_guypervisor;

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
        return status;
    }

    // Check if the vendor is Intel
    vendor_is_intel = virtualization::VendorIsIntel();
    if (!vendor_is_intel) {
        MDbgPrint("Failed because your vendor is not Intel.\n");
        status = STATUS_NOT_SUPPORTED;
        return status;
    }

    // Check if CPU supports VTx
    supports_vtx = virtualization::SupportsVtxOperation();
    if (!supports_vtx) {
        MDbgPrint("Failed because your processor does not support VTx.\n");
        status = STATUS_NOT_SUPPORTED;
        return status;
    }
    
    g_guypervisor =  new Device(DEVICE_NAME, DOS_DEVICE_NAME);
    status = g_guypervisor->InitDevice(pDriverObject);

    if (!NT_SUCCESS(status)) {
        MDbgPrint("Could not initiate device!\n");
        delete g_guypervisor;
        return status;
    }

    MDbgPrint("Successfully initiated device!\n");
    // Initialize WDF.

    // Create symbolic link to DOS Device
    status = g_guypervisor->CreateSymlink();
    if (!NT_SUCCESS(status)) {
        delete g_guypervisor;
        return status;
    }
    MDbgPrint("Successfully created symlink!\n");

	return status;
}

#undef DEVICE_NAME
#undef DOS_DEVICE_NAME
#undef HAS_CPUID_FLAG_MASK

