#pragma once

#include <wdm.h>

#include "virtualization.h"

#define IOCTL_SPOT CTL_CODE(FILE_DEVICE_UNKNOWN,\
                            virtualization::IoctlCodes::ENTER_VMX,\
                            METHOD_BUFFERED,\
                            FILE_ANY_ACCESS)

namespace IRPHandlers {
    
    DRIVER_DISPATCH CreateHandlerIRP; // IRP_MJ_CREATE
    DRIVER_DISPATCH CloseHandlerIRP;  // IRP_MJ_CLOSE
    DRIVER_DISPATCH WriteHandlerIRP;  // IRP_MJ_WRITE
    DRIVER_DISPATCH IOCTLHandlerIRP;  // IRP_MJ_DEVICE_CONTROL

    namespace IOCTLHandlers {
        DRIVER_DISPATCH IoctlSpotHandler;
    };

    DRIVER_DISPATCH GeneralHandlerIRP; // Handle the rest
};