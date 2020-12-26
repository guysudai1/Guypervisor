#ifndef __ERROR_CODES_H
#define __ERROR_CODES_H

#include <wdm.h>

const char* InstructionErrorToString(UINT32 errorCode);
const char* BasicExitReasonToString(UINT32 basicExitReason);

#endif /* __ERROR_CODES_H */