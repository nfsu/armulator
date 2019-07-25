#pragma once
#include "psr.hpp"
#include "types.hpp"

namespace arm {

	enum Register : u8 {

		r0, r1, r2, r3,
		r4, r5, r6, r7,
		r8, r9, r10, r11,
		r12, r13, r14, r15,

		count,

		lr = r13,	//Link register
		sp = r14,	//Stack pointer
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
	};

}