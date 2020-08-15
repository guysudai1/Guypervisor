#pragma once

#include <ntifs.h>
#include <wdf.h>

	

void* operator new (size_t count);
void operator delete(void* ptr);
void operator delete(void* ptr, unsigned __int64 size);
extern "C" DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD EventDeviceAdd;