#pragma once
#include "../types.hpp"

namespace arm::thumb {

	//A 3-bit lower register
	enum LoReg : u8 {
		r0, r1, r2, r3,
		r4, r5, r6, r7
	};

	//A 3-bit high register
	enum HiReg : u8 {
		r8, r9, r10, r11,
		r12, r13, r14, r15
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
	union RegOp0b {

		struct {

			TI Rd : 3;		//Destination register
			TI Rs : 3;		//Source register
			TI op : 10;		//Opcode

		};

		TI v;

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
	union RegOp3b {

		struct {

			TI Rd : 3;		//Destination register
			TI Rs : 3;		//Source register
			TI Rni : 3;		//Rn or intermediate
			TI op : 7;		//Opcode

		};

		TI v;

	};


	//An instruction executed on a destination and source register with an intermediate value;
	//
	//OPCODE Rd, Rs, #i
	//
	//Used for: Low register 8-bit LSL, LSR, ASR, ST/LD instructions
	//
	union RegOp5b {

		struct {

			TI Rd : 3;		//Destination register
			TI Rs : 3;		//Source register
			TI i : 5;		//Intermediate
			TI op : 5;		//Opcode

		};

		TI v;

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
	union RegOp8b {

		struct {

			TI i : 8;		//Intermediate
			TI Rd : 3;		//Destination register
			TI op : 5;		//Opcode

		};

		TI v;

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