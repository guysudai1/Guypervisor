#pragma once

#include <ntifs.h>
#include <wdf.h>

inline void acquire_max_phys_addr(PHYSICAL_ADDRESS& maxPhys);