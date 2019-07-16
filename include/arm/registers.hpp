#pragma once
#include "psr.hpp"
#include "types.hpp"

namespace arm {

	enum Register : Value4 {

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

	private:

		static constexpr u8 mapping[][Register::count] = {
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },			//SYS and USR
			{ 0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 15 },			//FIQ
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 23, 24, 15 },			//IRQ
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 25, 26, 15 },			//SVC
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 27, 28, 15 },			//ABT
			{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 29, 30, 15 }			//UND
		};

	public:

		u32 registers[38] {
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,Mode::USR,
			0,0,0,0,0,0
		};

		struct {
			u32 sysUsr[15];
			u32 pc;
			u32 fiq[7];
			u32 irq[2];
			u32 svc[2];
			u32 abt[2];
			u32 und[2];
			PSR cpsr;
			PSR spsr[6];
		};

		u32 &operator[](Register r) {
			return registers[mapping[Mode::toId(cpsr.mode)][r]];
		}

		PSR &getSpsr() {
			return spsr[Mode::toId(cpsr.mode)];
		}
	};

}