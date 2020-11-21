#ifndef __CUSTOM_STATUS_CODES_H
#define __CUSTOM_STATUS_CODES_H

#include <wdm.h>

#define BASE_ERROR 0xC0133700

/*
 * Memory allocation failure
 */
#define STATUS_FAILED_TO_ALLOCATE_MEMORY NT_ERROR(BASE_ERROR + 0x1)

/*
 * VM Instructions failure
 */
#define STATUS_FAILED_VMXON NT_ERROR(BASE_ERROR + 0x10)
#define STATUS_FAILED_VMXOFF NT_ERROR(BASE_ERROR + 0x11)
#define STATUS_FAILED_VMPTRLD NT_ERROR(BASE_ERROR + 0x12)
#define STATUS_FAILED_VMCLEAR NT_ERROR(BASE_ERROR + 0x13)

#endif
