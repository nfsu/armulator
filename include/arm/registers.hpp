#pragma once
#include "psr.hpp"
#include "types.hpp"
#include "exception.hpp"

namespace arm {

	enum Register : u8 {

		r0, r1, r2, r3,
		r4, r5, r6, r7,
		r8, r9, r10, r11,
		r12, r13, r14, r15,

		count,

		sp = r13,	//Stack pointer
		lr = r14,	//Link register
		pc = r15	//Program counter

	};

	union Registers {

	public:

		static constexpr u8 mapping[][Register::count] = {
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },			//SYS and USR
			{ 0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 15 },			//FIQ
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 23, 24, 15 },			//IRQ
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 25, 26, 15 },			//SVC
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 27, 28, 15 },			//ABT
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 29, 30, 15 }			//UND
		};

		u32 registers[40]{};

		struct {

			u32 loReg[8];
			u32 nonFiq[5];
			u32 sysUsr[2];
			u32 pc;
			u32 fiq[7];
			u32 irq[2];
			u32 svc[2];
			u32 abt[2];
			u32 und[2];

			PSR cpsr;
			PSR spsr[6];

			u32 ir;			//Instruction register
			u32 nir;		//Next instruction register
		};

		PSR &getSpsr() {
			return spsr[Mode::toId(cpsr.mode())];
		}

		template<Exception e>
		void exception() {

			constexpr Mode::E mode = Mode::E(u32(e) >> 8);

			if constexpr (mode == Mode::FIQ) {
				if (cpsr.disableFIQ())
					return;
			}
			else if constexpr (mode == Mode::IRQ)
				if (cpsr.disableIRQ())
					return;

			spsr[Mode::toId(mode)] = cpsr;			//Save cpsr
			cpsr.mode(mode);						//Set mode

			//Save next instruction in link register
			registers[mapping[mode][Register::lr]] = pc - (4 - cpsr.thumb() * 2);

			cpsr.clearThumb();						//Switch to arm mode
			cpsr.setIrq();							//Prevent IRQ interrupts

			if constexpr (mode == Mode::FIQ)
				cpsr.setFiq();						//Prevent FIQ interrupts

			pc = u32(e) & 0xFF;							//Jump to vector address

		}
	};

}