#pragma once
#include "registers.hpp"

namespace arm {

	#define __ALLOW_DEBUG__

	struct Armulator {

	public:

		enum class DebugLevel : u8 {
			NONE,
			MODIFIED,
			ALL
		};

		#ifdef __ALLOW_DEBUG__

			Registers p;

			void printModified();	//Print modified registers

		#endif

		Registers r;
		Buffer rom;

		DebugLevel debugLevel;

		Armulator(DebugLevel debug, const Buffer &rom, u32 entry, bool thumb, Mode::E mode = Mode::USR);

		bool step();	//Performs a 'step' (one operation)
		void wait();	//Performs operations until there are none left

		void print();			//Print all registers
		void printPSR(PSR psr);	//Print the PSR

	private:

		u32 stepThumb(const u8 *mapping, bool &setConditionCodes);
		u32 stepArm(const u8 *mapping, bool &setConditionCodes);

	};

}