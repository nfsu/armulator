#pragma once
#include "registers.hpp"
#include "stack.hpp"

//TODO: Add a callback before and after each step to allow for interception
//		Allows debugging/printing way more cleanly

namespace arm {

	#define __ALLOW_DEBUG__
	#define __INLINE__  __forceinline

	//!ARM7 emulator
	struct Armulator {

	public:

		enum Version {
			ARM7TDMI,
			ARM9TDMI
		};

		//Create the armulator to run the specified rom
		//@param[in] ranges; the memory ranges the armulator should map and use
		//@param[in] debug; how to handle printing each step of code
		//@param[in] rom; the rom data that has to be emulated
		//@param[in] entry; where the assembly entrypoint starts.
		//					4-byte aligned in ARM mode, 2-byte in thumb mode. 
		//					Add 1 to the address if thumb mode is enabled.
		//@param[in] mode; in what mode the emulator is launched (default = user)

		Armulator(const List<Memory32::Range> &ranges, u32 entry, Mode::E mode = Mode::USR);
		~Armulator() = default;

		Armulator(const Armulator&) = delete;
		Armulator(Armulator&&) = delete;
		Armulator &operator=(const Armulator&) = delete;
		Armulator &operator=(Armulator&&) = delete;

		void wait(Version v);	//Performs operations until there are none left

		void print();			//Print all registers
		void printPSR(PSR psr);	//Print the PSR

		Registers r;

		Memory32 memory;
		Stack32<0, 0> stack;

	private:

		bool init = false;

	};

}