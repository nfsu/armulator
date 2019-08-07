#pragma once
#include "registers.hpp"
#include "emu/memory.hpp"
#include "emu/stack.hpp"

namespace arm {

	//!ARM7 emulator
	//The armulator **RUNS IN THIS PROCESS** giving it access to memory allocated here as well
	//But only if it's allocated at 0xF0000000 -> 0xFFFFFFFF
	struct Armulator {

	public:

		struct VersionSpec {

			enum Version {
				v4 = 4, v5 = 5
			};

			enum Flags {

				T = 1 << 8,			//Thumb mode
				D = 1 << 9,			//Debugger
				M = 1 << 10,		//Multiplier
				I = 1 << 11,		//Debug operations through ICE

				TDMI = T | D | M | I

			};

		};

		enum Version {
			ARM7TDMI = VersionSpec::v4 | VersionSpec::TDMI,
			ARM9TDMI = VersionSpec::v5 | VersionSpec::TDMI,
		};

		enum DebugType {
			NONE = 0,				//Disable debugging
			PRINT_INSTRUCTION = 1,	//Print the instruction to the console
			PRINT_REGISTERS = 2		//Output the registers to the console
		};

		using Memory = emu::Memory32<0x80000000>;
		using Stack = emu::Stack<Memory, u32>;

		//Create the armulator to run the specified rom
		//@param[in] ranges; the memory ranges the armulator should map and use
		//@param[in] debug; how to handle printing each step of code
		//@param[in] rom; the rom data that has to be emulated
		//@param[in] entry; where the assembly entrypoint starts.
		//					4-byte aligned in ARM mode, 2-byte in thumb mode. 
		//					Add 1 to the address if thumb mode is enabled.
		//@param[in] mode; in what mode the emulator is launched (default = user)

		Armulator(const List<Memory::Range> &ranges);
		~Armulator() = default;

		Armulator(const Armulator&) = delete;
		Armulator(Armulator&&) = delete;
		Armulator &operator=(const Armulator&) = delete;
		Armulator &operator=(Armulator&&) = delete;

		static void print(Registers &r);			//Print all registers
		static void printPSR(PSR psr);				//Print the PSR

		Registers r;
		Memory memory;

	private:

		bool init = false;

	};

}