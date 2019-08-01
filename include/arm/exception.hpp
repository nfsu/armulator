#pragma once
#include "mode.hpp"

namespace arm {

	//Lower byte: Exception address
	//Other bytes: Mode
	enum class Exception {
		RESET = 0x0 | (Mode::SVC << 8),
		UND = 0x4 | (Mode::UND << 8),
		SWI = 0x8 | (Mode::SVC << 8),
		PREFETCH_ABORT = 0xC | (Mode::ABT << 8),
		DATA_ABORT = 0x10 | (Mode::ABT << 8),
		IRQ = 0x18 | (Mode::IRQ << 8),
		FIQ = 0x1C | (Mode::FIQ << 8)
	};

}