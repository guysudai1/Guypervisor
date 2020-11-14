#ifndef __PRINT_H
#define __PRINT_H

#include <wdm.h>
#include <ntstrsafe.h>

template <typename ...Args>
inline NTSTATUS MDbgPrint(const char* fmt, Args... args) {
	// Stores format strings
	char fmtString[256] = { 0 };
	NTSTATUS status = STATUS_SUCCESS;

	RtlStringCchCatA(fmtString, sizeof(fmtString), "[Guypervisor] ");
	RtlStringCchCatA(fmtString, sizeof(fmtString), fmt);

	DbgPrint(fmtString, args...);

	return status;
}

#endif /* __PRINT_H */
