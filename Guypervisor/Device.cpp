#include "Device.h"
#include "print.h"

extern Device* guyPervisor;


NTSTATUS Device::AddFunctions(PDRIVER_OBJECT pDriverObject)
{
    NTSTATUS status = STATUS_SUCCESS;

    for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; ++i) {
        pDriverObject->MajorFunction[i] = IRPHandlers::GeneralHandlerIRP;
    }

    pDriverObject->MajorFunction[IRP_MJ_CREATE] = IRPHandlers::CreateHandlerIRP;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = IRPHandlers::CloseHandlerIRP;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IRPHandlers::IOCTLHandlerIRP;
    pDriverObject->MajorFunction[IRP_MJ_WRITE] = IRPHandlers::WriteHandlerIRP;

    return status;
}

void Device::DriverUnload(PDRIVER_OBJECT pDeviceObject)
{
    UNREFERENCED_PARAMETER(pDeviceObject);

    MDbgPrint("Unloading driver...");
    delete guyPervisor;
}

NTSTATUS Device::InitDevice(PDRIVER_OBJECT pDriverObject)
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
   
    return status;
}

NTSTATUS Device::CreateSymlink()
{
    // Create symlink from \Device\Guypervisor to \DosDevices\Guypervisor

    NTSTATUS status = STATUS_SUCCESS;
    
    // Delete symlink
    IoDeleteSymbolicLink(&dosDeviceName);

    // Create new symlink
    status = IoCreateSymbolicLink(&dosDeviceName, &deviceName);
    if (!NT_SUCCESS(status)) {
        MDbgPrint("Failed creating a symbolic link with error: %d", status);
    }
    return status;
}

void Device::operator delete(void* p, unsigned __int64 size)
{
    UNREFERENCED_PARAMETER(size);
    Device* ptr_to_delete = static_cast<Device*>(p);
    
    IoDeleteDevice(ptr_to_delete->pDeviceObject);
    delete p;
    // IoDeleteDevice(
    // this->pDeviceObject);
}

Device::~Device()
{
    // delet this
}

Device::Device(const wchar_t* deviceName, const wchar_t* dosDeviceName)
{
    this->pDeviceObject = nullptr;

	// Initiatlize UNICODE_STRING objects.
	RtlInitUnicodeString(&this->deviceName, deviceName);
	RtlInitUnicodeString(&this->dosDeviceName, dosDeviceName); 

}


