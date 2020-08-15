#pragma once

#include <ntifs.h>
#include <wdf.h>

//typedef NTSTATUS(*IRPHandlerFunction)(_In_ PDEVICE_OBJECT, _In_ PIRP);
#define IOCTL_SPOT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

namespace IRPHandlers {
    // Handle no IRP
    DRIVER_DISPATCH GeneralHandlerIRP; // Rest
    DRIVER_DISPATCH CreateHandlerIRP; // IRP_MJ_CREATE
    DRIVER_DISPATCH CloseHandlerIRP; // IRP_MJ_CLOSE
    DRIVER_DISPATCH WriteHandlerIRP; // IRP_MJ_WRITE
    DRIVER_DISPATCH IOCTLHandlerIRP; // IRP_MJ_DEVICE_CONTROL
    namespace IOCTLHandlers {
        DRIVER_DISPATCH IoctlSpotHandler;
    };
};