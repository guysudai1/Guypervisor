#pragma once

#include <ntifs.h>
#include <wdf.h>
#include <ntstrsafe.h>

template <typename ...Args>
NTSTATUS MDbgPrint(const char* fmt, Args... args) {
	// Stores format strings
	char fmtString[100] = { 0 };
	NTSTATUS status = STATUS_SUCCESS;

	RtlStringCchCatA(fmtString, sizeof(fmtString), "[Guypervisor] ");
	RtlStringCchCatA(fmtString, sizeof(fmtString), fmt);


	DbgPrint(fmtString, args...);

	return status;
}
