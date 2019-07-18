#pragma once
#include "mode.hpp"

namespace arm {

	union PSR {

		struct {

			//Control bits

			Mode::E mode : 5;			//The execution mode
			bool thumb : 1;				//T; if thumb mode is used
			bool disableFiq : 1;		//F: if FIQ is disabled
			bool disableIrq : 1;		//I: if IRQ is disabled

			//Unused for ARM7/9TDMI (20 bits)
			//Padding 2 bytes + 4 bits

			u8 p0[2];
			u8 p1 : 4;

			//Condition flags

			bool overflow : 1;			//V: the last operation caused a signed overflow
			bool carry : 1;				//C: the last operation caused an unsigned overflow
			bool zero : 1;				//Z: the last operation was zero
			bool negative : 1;			//N: the last operation was negative

		};

		u32 value;

	};

}