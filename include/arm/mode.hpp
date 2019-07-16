#pragma once
#include "types/types.hpp"

namespace arm {

	struct Mode {

	private:

		//Lookup table for mode id from mode enum
		static constexpr u8 modeToId[] = {
			6,6,6,6, 6,6,6,6, 6,6,6,6, 6,6,6,6,			//Legacy ARM6 modes
			0,1,2,3, 6,6,6,4, 6,6,6,5, 6,6,6,0			//USR, FIQ, IRQ, ABT, UND, SYS
		};

	public:

		enum E : Value5 {
			USR = 0x10,
			FIQ = 0x11,
			IRQ = 0x12,
			SVC = 0x13,
			ABT = 0x17,
			UND = 0x1B,
			SYS = 0x1F
		};

		static bool isValid(E m) {
			return (m & USR) && ((m & 3) == 3 || m < SVC);
		}

		static u8 toId(E m) {
			return modeToId[m];
		}

	};

}