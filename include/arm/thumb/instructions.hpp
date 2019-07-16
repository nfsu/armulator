#pragma once
#include "reg_op.hpp"

namespace arm::thumb {

	//First 5-bits of an opcode (RegOp5b)
	enum OpCode5 : Value5 {

		LSL			= 0b0'0000,
		LSR			= 0b0'0001,
		ASR			= 0b0'0010,
		ADD_SUB		= 0b0'0011,
		MOV			= 0b0'0100,
		CMP			= 0b0'0101,
		ADD			= 0b0'0110,
		SUB			= 0b0'0111,
		ALU_HI_BX	= 0b0'1000,
		LDR_PC		= 0b0'1001,
		ST			= 0b0'1010,
		LD			= 0b0'1011,
		STRi		= 0b0'1100,
		LDRi		= 0b0'1101,
		STRBi		= 0b0'1110,
		LDRBi		= 0b0'1111,
		STRHi		= 0b1'0000,
		LDRHi		= 0b1'0001,
		STR_SP		= 0b1'0010,
		LDR_SP		= 0b1'0011,
		ADD_PC		= 0b1'0100,
		ADD_SP		= 0b1'0101,
		INCR_SP		= 0b1'0110,
		PUSH_POP	= 0b1'0111,
		STRMIA		= 0b1'1000,
		LDMIA		= 0b1'1001,
		B0			= 0b1'1010,		B1			= 0b1'1011,
		B			= 0b1'1100,
		BLH			= 0b1'1110,		BLL			= 0b1'1111

	};

	//Extended 7-bit opcodes (RegOp3b)
	enum OpCode7 : Value7 {

		ADD_R		= 0b0'0011'00,
		SUB_R		= 0b0'0011'01,
		ADD_3B		= 0b0'0011'10,
		SUB_3B		= 0b0'0011'11,

		STR			= 0b0'1010'00,
		STRH		= 0b0'1010'01,
		STRB		= 0b0'1010'10,
		LDSB		= 0b0'1010'11,
		LDR			= 0b0'1011'00,
		LDRH		= 0b0'1011'01,
		LDRB		= 0b0'1011'10,
		LDSH		= 0b0'1011'11

	};

	//Extended 10-bit opcodes (RegOp0b)
	enum OpCode10 : Value10 {

		AND			= 0b0'0011'0'0000,
		EOR			= 0b0'0011'0'0001,
		LSL_R		= 0b0'0011'0'0010,
		LSR_R		= 0b0'0011'0'0011,
		ASR_R		= 0b0'0011'0'0100,
		ADC			= 0b0'0011'0'0101,
		SBC			= 0b0'0011'0'0110,
		ROR			= 0b0'0011'0'0111,
		TST			= 0b0'0011'0'1000,
		NEG			= 0b0'0011'0'1001,
		CMP_R		= 0b0'0011'0'1010,
		CMN			= 0b0'0011'0'1011,
		ORR			= 0b0'0011'0'1100,
		MUL			= 0b0'0011'0'1101,
		BIC			= 0b0'0011'0'1110,
		MVN			= 0b0'0011'0'1111,
		ADD_LO_HI	= 0b0'0011'1'0001,
		ADD_HI_LO	= 0b0'0011'1'0010,
		ADD_HI_HI	= 0b0'0011'1'0011,
		CMP_LO_HI	= 0b0'0011'1'0101,
		CMP_HI_LO	= 0b0'0011'1'0110,
		CMP_HI_HI	= 0b0'0011'1'0111,
		MOV_LO_HI	= 0b0'0011'1'1001,
		MOV_HI_LO	= 0b0'0011'1'1010,
		MOV_HI_HI	= 0b0'0011'1'1011,
		BX_LO		= 0b0'0011'1'1100,
		BX_HI		= 0b0'0011'1'1101

	};

	//RegOp0b

	static constexpr TI and (LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, AND }.v; }
	static constexpr TI eor(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, EOR }.v; }
	static constexpr TI lsl(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, LSL_R }.v; }
	static constexpr TI lsr(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, LSR_R }.v; }
	static constexpr TI asr(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, ASR_R }.v; }
	static constexpr TI adc(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, ADC }.v; }
	static constexpr TI sbc(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, SBC }.v; }
	static constexpr TI ror(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, ROR }.v; }
	static constexpr TI tst(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, TST }.v; }
	static constexpr TI neg(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, NEG }.v; }
	static constexpr TI cmp(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, CMP_R }.v; }
	static constexpr TI cmn(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, CMN }.v; }
	static constexpr TI orr(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, ORR }.v; }
	static constexpr TI mul(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, MUL }.v; }
	static constexpr TI bic(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, BIC }.v; }
	static constexpr TI mvn(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, MVN }.v; }
	static constexpr TI add(LoReg Rd, HiReg Hs) { return RegOp0b{ Rd, Hs, ADD_LO_HI }.v; }
	static constexpr TI add(HiReg Hd, LoReg Rs) { return RegOp0b{ Hd, Rs, ADD_HI_LO }.v; }
	static constexpr TI add(HiReg Hd, HiReg Hs) { return RegOp0b{ Hd, Hs, ADD_HI_HI }.v; }
	static constexpr TI cmp(LoReg Rd, HiReg Hs) { return RegOp0b{ Rd, Hs, CMP_LO_HI }.v; }
	static constexpr TI cmp(HiReg Hd, LoReg Rs) { return RegOp0b{ Hd, Rs, CMP_HI_LO }.v; }
	static constexpr TI cmp(HiReg Hd, HiReg Hs) { return RegOp0b{ Hd, Hs, CMP_HI_HI }.v; }
	static constexpr TI mov(LoReg Rd, HiReg Hs) { return RegOp0b{ Rd, Hs, MOV_LO_HI }.v; }
	static constexpr TI mov(HiReg Hd, LoReg Rs) { return RegOp0b{ Hd, Rs, MOV_HI_LO }.v; }
	static constexpr TI mov(HiReg Hd, HiReg Hs) { return RegOp0b{ Hd, Hs, MOV_HI_HI }.v; }
	static constexpr TI bx(LoReg Rs) { return RegOp0b{ 0, Rs, BX_LO }.v; }
	static constexpr TI bx(HiReg Hs) { return RegOp0b{ 0, Hs, BX_HI }.v; }

	//RegOp3b

	static constexpr TI add(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, ADD_R }.v; }
	static constexpr TI sub(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, SUB_R }.v; }
	static constexpr TI add(LoReg Rd, LoReg Rs, Value3 i) { return RegOp3b{ Rd, Rs, i, ADD_3B }.v; }
	static constexpr TI sub(LoReg Rd, LoReg Rs, Value3 i) { return RegOp3b{ Rd, Rs, i, SUB_3B }.v; }

	static constexpr TI str(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, STR }.v; }
	static constexpr TI strh(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, STRH }.v; }
	static constexpr TI strb(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, STRB }.v; }
	static constexpr TI ldsb(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, LDSB }.v; }
	static constexpr TI ldr(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, LDR }.v; }
	static constexpr TI ldrh(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, LDRH }.v; }
	static constexpr TI ldrb(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, LDRB }.v; }
	static constexpr TI ldsh(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, LDSH }.v; }

	//RegOp5b

	static constexpr TI lsl(LoReg Rd, LoReg Rs, Value5 i) { return RegOp5b{ Rd, Rs, i, LSL }.v; }
	static constexpr TI lsr(LoReg Rd, LoReg Rs, Value5 i) { return RegOp5b{ Rd, Rs, i, LSR }.v; }
	static constexpr TI asr(LoReg Rd, LoReg Rs, Value5 i) { return RegOp5b{ Rd, Rs, i, ASR }.v; }

	static constexpr TI str(LoReg Rd, LoReg Rs, Value7 offset) { return RegOp3b{ Rd, Rs, TI(offset >> 2), STRi }.v; }
	static constexpr TI ldr(LoReg Rd, LoReg Rs, Value7 offset) { return RegOp3b{ Rd, Rs, TI(offset >> 2), LDRi }.v; }
	static constexpr TI strb(LoReg Rd, LoReg Rs, Value5 offset) { return RegOp3b{ Rd, Rs, offset, STRBi }.v; }
	static constexpr TI ldrb(LoReg Rd, LoReg Rs, Value5 offset) { return RegOp3b{ Rd, Rs, offset, LDRBi }.v; }
	static constexpr TI strh(LoReg Rd, LoReg Rs, Value6 offset) { return RegOp3b{ Rd, Rs, TI(offset >> 1), STRHi }.v; }
	static constexpr TI ldrh(LoReg Rd, LoReg Rs, Value6 offset) { return RegOp3b{ Rd, Rs, TI(offset >> 1), LDRHi }.v; }

	//Reg0p7b

	//TODO: ADD SP, +/- 128
	//TODO: PUSH/POP

	//RegOp8b

	static constexpr TI mov(LoReg Rd, u8 i) { return RegOp8b{ i, Rd, MOV }.v; }
	static constexpr TI cmp(LoReg Rd, u8 i) { return RegOp8b{ i, Rd, CMP }.v; }
	static constexpr TI add(LoReg Rd, u8 i) { return RegOp8b{ i, Rd, ADD }.v; }
	static constexpr TI sub(LoReg Rd, u8 i) { return RegOp8b{ i, Rd, SUB }.v; }

	static constexpr TI ldrPc(LoReg Rd, Value10 offset) { return RegOp8b{ TI(offset >> 2), Rd, LDR_PC }.v;}

	static constexpr TI strSp(LoReg Rd, Value10 offset) { return RegOp8b{ TI(offset >> 2), Rd, STR_SP }.v; }
	static constexpr TI ldrSp(LoReg Rd, Value10 offset) { return RegOp8b{ TI(offset >> 2), Rd, LDR_SP }.v; }
	static constexpr TI addPc(LoReg Rd, Value10 offset) { return RegOp8b{ TI(offset >> 2), Rd, ADD_PC }.v; }
	static constexpr TI addSp(LoReg Rd, Value10 offset) { return RegOp8b{ TI(offset >> 2), Rd, ADD_SP }.v; }

}