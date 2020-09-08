#pragma once
#include "vmcs.h"

struct processorContext {
	VMCS* vmxon_region;
	VMCS* vmcs_region;
};

