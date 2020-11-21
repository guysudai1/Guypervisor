#ifndef __DEVICE_H
#define __DEVICE_H

#include <wdm.h>

#include "irp_handlers.h"

class Device {
public:

    static NTSTATUS AddFunctions(
        _In_ PDRIVER_OBJECT    pDriverObject
    );

    static void DriverUnload(
        _In_ PDRIVER_OBJECT    pDriverObject
    );

    // Initiates device object
    NTSTATUS InitDevice(
        _In_ PDRIVER_OBJECT		pDriverObject
    );

    // Creates symbolic link to device
    NTSTATUS CreateSymlink();

    void operator delete (void* p, unsigned __int64 size);
    
    Device(const wchar_t* deviceName, const wchar_t* dosDeviceName);
    ~Device();

private:
    UNICODE_STRING deviceName;
    UNICODE_STRING dosDeviceName;
    PDEVICE_OBJECT pDeviceObject;
};

extern Device* kGuypervisor;

#endif /* __DEVICE_H */