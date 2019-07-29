#pragma once
#include "../types.hpp"
#include "opcodes.hpp"

namespace arm::thumb {

	//A 3-bit lower register
	enum LoReg : u8 {
		r0, r1, r2, r3,
		r4, r5, r6, r7
	};

	//A 3-bit high register
	enum HiReg : u8 {

		r8, r9, r10, r11,
		r12, r13, r14, r15,

		sp = HiReg::r13,
		lr = HiReg::r14,
		pc = HiReg::r15
	};

	//A thumb instruction
	using TI = u16;

	//An instruction executed on a destination and source register;
	//
	//OPCODE Rd, Rs
	//
	//Used for: Low register AND, EOR, LSL, LSR, ASR, ADC, SBC, ROR,
	//			TST, NEG, CMP, CMN, ORR, MUL, BIC, MVN, BX
	//			High register ADD, MOV, BX
	//
	struct RegOp0b {

		TI v;

		RegOp0b(TI Rd, TI Rs, TI op): v(Rd | (Rs << 3) | (op << 6)) {}

		__forceinline u32 Rd() const { return v & 7; }			//Destination register
		__forceinline u32 Rs() const { return (v >> 3) & 7; }	//Source register
		__forceinline u32 op() const { return v >> 6; }			//Op code

	};

	//An instruction executed on a destination and source register with an intermediate register/value;
	//
	//OPCODE Rd, Rs, Rn
	// and
	//OPCODE Rd, Rs, #i
	//
	//Used for: Low register ADD, SUB, ST/LD instructions
	//			Low register 3-bit ADD, SUB
	//
	struct RegOp3b {

		TI v;

		RegOp3b(TI Rd, TI Rs, TI Rni, TI op): v(Rd | (Rs << 3) | (Rni << 6) | (op << 9)) {}

		__forceinline u32 Rd() const { return v & 7; }			//Destination register
		__forceinline u32 Rs() const { return (v >> 3) & 7; }	//Source register
		__forceinline u32 Rni() const { return (v >> 6) & 7; }	//Source register
		__forceinline u32 op() const { return v >> 9; }			//Op code

	};


	//An instruction executed on a destination and source register with an intermediate value;
	//
	//OPCODE Rd, Rs, #i
	//
	//Used for: Low register 8-bit LSL, LSR, ASR, ST/LD instructions
	//
	struct RegOp5b {

		TI v;

		RegOp5b(TI Rd, TI Rs, TI i, TI op): v(Rd | (Rs << 3) | (i << 6) | (op << 11)) {}

		__forceinline u32 Rd() const { return v & 7; }			//Destination register
		__forceinline u32 Rs() const { return (v >> 3) & 7; }	//Source register
		__forceinline u32 i() const { return (v >> 6) & 0x1F; }	//Intermediate
		__forceinline u32 op() const { return v >> 11; }		//Op code

	};

	//A stack modifying instruction executed on a destination and source register;
	//
	//Used for: ADD SP, #i; PUSH/POP
	//
	union RegOp7b {

		struct {

			TI i : 7;		//Intermediate
			TI b : 1;		//Additional opcode bit
			TI op : 8;		//Opcode

		};

		TI v;

	};

	//An instruction executed on a destination register with an intermediate value;
	//
	//OPCODE Rd, #i
	//
	//Used for: Low register 8-bit MOV, CMP, ADD, SUB,  LDR Rd, [PC, #), STRMIA/LDMIA
	//
	struct RegOp8b {

		TI v;

		RegOp8b(TI i, TI Rd, TI op): v(i | (Rd << 8) | (op << 11)) {}

		__forceinline u32 i() const { return v & 0xFF; }		//Intermediate
		__forceinline u32 Rd() const { return (v >> 8) & 7; }	//Destination register
		__forceinline u32 op() const { return v >> 11; }		//Op code

	};

	//A conditional branch & software interrupt instruction;
	//
	//SWI #i
	// and
	//B{cond} label
	//
	union RegOp12b {

		struct {

			TI i : 8;		//Intermediate
			TI cond : 4;	//Execute condition
			TI op : 4;		//Opcode

		};

		TI v;

	};

}