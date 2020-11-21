#include "Device.h"

#include "print.h"


NTSTATUS Device::AddFunctions(_In_ PDRIVER_OBJECT pDriverObject)
{
    NTSTATUS status = STATUS_SUCCESS;

    for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; ++i) {
        pDriverObject->MajorFunction[i] = irp_handlers::GeneralHandlerIRP;
    }

    pDriverObject->MajorFunction[IRP_MJ_CREATE] = irp_handlers::CreateHandlerIRP;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = irp_handlers::CloseHandlerIRP;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = irp_handlers::IOCTLHandlerIRP;
    pDriverObject->MajorFunction[IRP_MJ_WRITE] = irp_handlers::WriteHandlerIRP;

    return status;
}

void Device::DriverUnload(_In_ PDRIVER_OBJECT pDeviceObject)
{
    UNREFERENCED_PARAMETER(pDeviceObject);

    MDbgPrint("Unloading driver...");
    delete kGuypervisor;
}

NTSTATUS Device::InitDevice(_In_ PDRIVER_OBJECT pDriverObject)
{
    // Initiates device with IoCreateDevice
    NTSTATUS status;

    status = IoCreateDevice(pDriverObject,               // PDRIVER_OBJECT  DriverObject
        0,                          // ULONG           DeviceExtensionSize
        &deviceName,                // PUNICODE_STRING DeviceName
        FILE_DEVICE_UNKNOWN,        // DEVICE_TYPE     DeviceType
        FILE_DEVICE_SECURE_OPEN,    // ULONG           DeviceCharacteristics
        FALSE,                      // BOOLEAN         Exclusive
        &this->pDeviceObject);

    if (!NT_SUCCESS(status)) {
        MDbgPrint("Failed creating IO device with error: %d", status);
    }

    pDriverObject->DriverUnload = Device::DriverUnload;

    this->AddFunctions(pDriverObject);

    return status;
}

NTSTATUS Device::CreateSymlink()
{
    // Create symlink from \Device\Guypervisor to \DosDevices\Guypervisor

    NTSTATUS status = STATUS_SUCCESS;
    
    // Delete symlink
    status = IoDeleteSymbolicLink(&this->dosDeviceName);
    
    if (!NT_SUCCESS(status))
    {
        MDbgPrint("Failed to delete symbol link %s with error: %d\n", this->dosDeviceName, status);
        goto cleanup;
    }

    // Create new symlink
    status = IoCreateSymbolicLink(&this->dosDeviceName, &this->deviceName);
    if (!NT_SUCCESS(status))
    {
        MDbgPrint("Failed to create symbol link %s with error: %d\n", this->dosDeviceName, status);
        goto cleanup;
    }

cleanup:
    return status;
}

void Device::operator delete(void* p, unsigned __int64 size)
{
    UNREFERENCED_PARAMETER(size);
    Device* ptr_to_delete = static_cast<Device*>(p);
    
    IoDeleteDevice(ptr_to_delete->pDeviceObject);
    delete p;
}

Device::~Device()
{
    // delet this
}

Device::Device(const wchar_t* deviceName, 
               const wchar_t* dosDeviceName)
{
    this->pDeviceObject = nullptr;

	// Initiatlize UNICODE_STRING objects.
	RtlInitUnicodeString(&this->deviceName, deviceName);
	RtlInitUnicodeString(&this->dosDeviceName, dosDeviceName); 

}


