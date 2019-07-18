#pragma once
#include "types/types.hpp"

namespace arm {

	//Shift right, maintain sign
	static inline u32 asr(u32 a, u32 b) {
		return a & i32_MIN ? ~(~a >> b) : a >> b;
	}

}