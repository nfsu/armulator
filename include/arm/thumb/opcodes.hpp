#pragma once
#include "types/types.hpp"

namespace arm::thumb {

	//First 5-bits of an opcode (RegOp5b)
	enum OpCode5 : u8 {

		LSL = 0b0'0000,
		LSR = 0b0'0001,
		ASR = 0b0'0010,
		ADD_SUB = 0b0'0011,
		MOV = 0b0'0100,
		CMP = 0b0'0101,
		ADD = 0b0'0110,
		SUB = 0b0'0111,
		ALU_HI_BX = 0b0'1000,
		LDR_PC = 0b0'1001,
		ST = 0b0'1010,
		LD = 0b0'1011,
		STRi = 0b0'1100,
		LDRi = 0b0'1101,
		STRBi = 0b0'1110,
		LDRBi = 0b0'1111,
		STRHi = 0b1'0000,
		LDRHi = 0b1'0001,
		STR_SP = 0b1'0010,
		LDR_SP = 0b1'0011,
		ADD_PC = 0b1'0100,
		ADD_SP = 0b1'0101,
		INCR_SP = 0b1'0110,
		PUSH_POP = 0b1'0111,
		STMIA = 0b1'1000,
		LDMIA = 0b1'1001,
		B0 = 0b1'1010, B1 = 0b1'1011,
		B = 0b1'1100,
		BLH = 0b1'1110, BLL = 0b1'1111

	};

	//Extended 7-bit opcodes (RegOp3b)
	enum OpCode7 : u8 {

		ADD_R = 0b0'0011'00,
		SUB_R = 0b0'0011'01,
		ADD_3B = 0b0'0011'10,
		SUB_3B = 0b0'0011'11,

		STR = 0b0'1010'00,
		STRH = 0b0'1010'01,
		STRB = 0b0'1010'10,
		LDSB = 0b0'1010'11,
		LDR = 0b0'1011'00,
		LDRH = 0b0'1011'01,
		LDRB = 0b0'1011'10,
		LDSH = 0b0'1011'11,

		ADD_TO_SP = 0b1'0110'00,
		PUSH = 0b1'0110'10,
		POP = 0b1'0111'10

	};

	//First 8-bits of an opcode (
	enum OpCode8 : u8 {

		BEQ = 0b1'101'0000,
		BNE,
		BCS,
		BCC,
		BMI,
		BPL,
		BVS,
		BVC,
		BHI,
		BLS,
		BGE,
		BLT,
		BGT,
		BLE,
		BAL,

		SWI


	};

	//Extended 10-bit opcodes (RegOp0b)
	enum OpCode10 : u16 {

		AND = 0b0'1000'0'0000,
		EOR = 0b0'1000'0'0001,
		LSL_R = 0b0'1000'0'0010,
		LSR_R = 0b0'1000'0'0011,
		ASR_R = 0b0'1000'0'0100,
		ADC = 0b0'1000'0'0101,
		SBC = 0b0'1000'0'0110,
		ROR = 0b0'1000'0'0111,
		TST = 0b0'1000'0'1000,
		NEG = 0b0'1000'0'1001,
		CMP_R = 0b0'1000'0'1010,
		CMN = 0b0'1000'0'1011,
		ORR = 0b0'1000'0'1100,
		MUL = 0b0'1000'0'1101,
		BIC = 0b0'1000'0'1110,
		MVN = 0b0'1000'0'1111,
		ADD_LO_HI = 0b0'1000'1'0001,
		ADD_HI_LO = 0b0'1000'1'0010,
		ADD_HI_HI = 0b0'1000'1'0011,
		CMP_LO_HI = 0b0'1000'1'0101,
		CMP_HI_LO = 0b0'1000'1'0110,
		CMP_HI_HI = 0b0'1000'1'0111,
		MOV_LO_HI = 0b0'1000'1'1001,
		MOV_HI_LO = 0b0'1000'1'1010,
		MOV_HI_HI = 0b0'1000'1'1011,
		BX_LO = 0b0'1000'1'1100,
		BX_HI = 0b0'1000'1'1101

	};

}