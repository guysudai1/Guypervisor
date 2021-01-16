#include "msr.h"

namespace msr {
	unsigned long long ReadMsr(intel_e msr_num) {
		return __readmsr(static_cast<unsigned long>(msr_num));
	}
}