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

		static constexpr u8 names[][4] = {
			"m00", "m01", "m02", "m03",  "m04", "m05", "m06", "m07",
			"m08", "m09", "m0A", "m0B",  "m0C", "m0D", "m0E", "m0F",
			"USR", "FIQ", "IRQ", "SVC",	 "m14", "m15", "m16", "ABT",
			"m18", "m19", "m1A", "UND",  "m1C", "m1D", "m1E", "SYS"

		};

	public:

		enum E : u8 {
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

		static const u8 *name(E m) {
			return names[m];
		}

	};

}