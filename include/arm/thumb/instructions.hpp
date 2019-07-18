#pragma once
#include "reg_op.hpp"
#include "../condition.hpp"
#include "types/bitset.hpp"

namespace arm::thumb {

	//First 5-bits of an opcode (RegOp5b)
	enum OpCode5 : u8 {

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
		STMIA		= 0b1'1000,
		LDMIA		= 0b1'1001,
		B0			= 0b1'1010,		B1			= 0b1'1011,
		B			= 0b1'1100,
		BLH			= 0b1'1110,		BLL			= 0b1'1111

	};

	//Extended 7-bit opcodes (RegOp3b)
	enum OpCode7 : u8 {

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
		LDSH		= 0b0'1011'11,

		ADD_TO_SP = 0b1'0110'00,
		PUSH = 0b1'0110'10,
		POP = 0b1'0111'10

	};

	//Extended 10-bit opcodes (RegOp0b)
	enum OpCode10 : u16 {

		AND			= 0b0'1000'0'0000,
		EOR			= 0b0'1000'0'0001,
		LSL_R		= 0b0'1000'0'0010,
		LSR_R		= 0b0'1000'0'0011,
		ASR_R		= 0b0'1000'0'0100,
		ADC			= 0b0'1000'0'0101,
		SBC			= 0b0'1000'0'0110,
		ROR			= 0b0'1000'0'0111,
		TST			= 0b0'1000'0'1000,
		NEG			= 0b0'1000'0'1001,
		CMP_R		= 0b0'1000'0'1010,
		CMN			= 0b0'1000'0'1011,
		ORR			= 0b0'1000'0'1100,
		MUL			= 0b0'1000'0'1101,
		BIC			= 0b0'1000'0'1110,
		MVN			= 0b0'1000'0'1111,
		ADD_LO_HI	= 0b0'1000'1'0001,
		ADD_HI_LO	= 0b0'1000'1'0010,
		ADD_HI_HI	= 0b0'1000'1'0011,
		CMP_LO_HI	= 0b0'1000'1'0101,
		CMP_HI_LO	= 0b0'1000'1'0110,
		CMP_HI_HI	= 0b0'1000'1'0111,
		MOV_LO_HI	= 0b0'1000'1'1001,
		MOV_HI_LO	= 0b0'1000'1'1010,
		MOV_HI_HI	= 0b0'1000'1'1011,
		BX_LO		= 0b0'1000'1'1100,
		BX_HI		= 0b0'1000'1'1101

	};

	//RegOp0b

	static inline TI and(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, AND }.v; }
	static inline TI eor(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, EOR }.v; }
	static inline TI lsl(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, LSL_R }.v; }
	static inline TI lsr(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, LSR_R }.v; }
	static inline TI asr(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, ASR_R }.v; }
	static inline TI adc(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, ADC }.v; }
	static inline TI sbc(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, SBC }.v; }
	static inline TI ror(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, ROR }.v; }
	static inline TI tst(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, TST }.v; }
	static inline TI neg(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, NEG }.v; }
	static inline TI cmp(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, CMP_R }.v; }
	static inline TI cmn(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, CMN }.v; }
	static inline TI orr(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, ORR }.v; }
	static inline TI mul(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, MUL }.v; }
	static inline TI bic(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, BIC }.v; }
	static inline TI mvn(LoReg Rd, LoReg Rs) { return RegOp0b{ Rd, Rs, MVN }.v; }
	static inline TI add(LoReg Rd, HiReg Hs) { return RegOp0b{ Rd, Hs, ADD_LO_HI }.v; }
	static inline TI add(HiReg Hd, LoReg Rs) { return RegOp0b{ Hd, Rs, ADD_HI_LO }.v; }
	static inline TI add(HiReg Hd, HiReg Hs) { return RegOp0b{ Hd, Hs, ADD_HI_HI }.v; }
	static inline TI cmp(LoReg Rd, HiReg Hs) { return RegOp0b{ Rd, Hs, CMP_LO_HI }.v; }
	static inline TI cmp(HiReg Hd, LoReg Rs) { return RegOp0b{ Hd, Rs, CMP_HI_LO }.v; }
	static inline TI cmp(HiReg Hd, HiReg Hs) { return RegOp0b{ Hd, Hs, CMP_HI_HI }.v; }
	static inline TI mov(LoReg Rd, HiReg Hs) { return RegOp0b{ Rd, Hs, MOV_LO_HI }.v; }
	static inline TI mov(HiReg Hd, LoReg Rs) { return RegOp0b{ Hd, Rs, MOV_HI_LO }.v; }
	static inline TI mov(HiReg Hd, HiReg Hs) { return RegOp0b{ Hd, Hs, MOV_HI_HI }.v; }
	static inline TI bx(LoReg Rs) { return RegOp0b{ 0, Rs, BX_LO }.v; }
	static inline TI bx(HiReg Hs) { return RegOp0b{ 0, Hs, BX_HI }.v; }

	//RegOp3b

	static inline TI add(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, ADD_R }.v; }
	static inline TI sub(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, SUB_R }.v; }
	static inline TI add(LoReg Rd, LoReg Rs, Value3 i) { return RegOp3b{ Rd, Rs, i.at(0), ADD_3B }.v; }
	static inline TI sub(LoReg Rd, LoReg Rs, Value3 i) { return RegOp3b{ Rd, Rs, i.at(0), SUB_3B }.v; }

	static inline TI str(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, STR }.v; }
	static inline TI strh(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, STRH }.v; }
	static inline TI strb(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, STRB }.v; }
	static inline TI ldsb(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, LDSB }.v; }
	static inline TI ldr(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, LDR }.v; }
	static inline TI ldrh(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, LDRH }.v; }
	static inline TI ldrb(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, LDRB }.v; }
	static inline TI ldsh(LoReg Rd, LoReg Rs, LoReg Rn) { return RegOp3b{ Rd, Rs, Rn, LDSH }.v; }

	//RegOp5b

	static inline TI lsl(LoReg Rd, LoReg Rs, Value5 i) { return RegOp5b{ Rd, Rs, TI(i), LSL }.v; }
	static inline TI lsr(LoReg Rd, LoReg Rs, Value5 i) { return RegOp5b{ Rd, Rs, TI(i), LSR }.v; }
	static inline TI asr(LoReg Rd, LoReg Rs, Value5 i) { return RegOp5b{ Rd, Rs, TI(i), ASR }.v; }

	static inline TI str(LoReg Rd, LoReg Rs, Value7 offset) { return RegOp3b{ Rd, Rs, TI(offset >> 2), STRi }.v; }
	static inline TI ldr(LoReg Rd, LoReg Rs, Value7 offset) { return RegOp3b{ Rd, Rs, TI(offset >> 2), LDRi }.v; }
	static inline TI strb(LoReg Rd, LoReg Rs, Value5 offset) { return RegOp3b{ Rd, Rs, TI(offset), STRBi }.v; }
	static inline TI ldrb(LoReg Rd, LoReg Rs, Value5 offset) { return RegOp3b{ Rd, Rs, TI(offset), LDRBi }.v; }
	static inline TI strh(LoReg Rd, LoReg Rs, Value6 offset) { return RegOp3b{ Rd, Rs, TI(offset >> 1), STRHi }.v; }
	static inline TI ldrh(LoReg Rd, LoReg Rs, Value6 offset) { return RegOp3b{ Rd, Rs, TI(offset >> 1), LDRHi }.v; }

	//Reg0p7b

	static inline TI addToSp(Value7 offset, bool negative){ return RegOp7b{ TI(offset), negative, ADD_TO_SP }.v; }

	static inline TI push(const oic::Bitset8<7> &Rs) { return RegOp7b{ Rs.at(0), false, PUSH }.v; }
	static inline TI pop(const oic::Bitset8<7> &Rs) { return RegOp7b{ Rs.at(0), false, POP }.v; }
	static inline TI pushLr(const oic::Bitset8<7> & Rs) { return RegOp7b { Rs.at(0), true, PUSH }.v; }
	static inline TI popPc(const oic::Bitset8<7> & Rs) { return RegOp7b { Rs.at(0), true, POP }.v; }

	//RegOp8b

	static inline TI mov(LoReg Rd, u8 i) { return RegOp8b{ TI(i), Rd, MOV }.v; }
	static inline TI cmp(LoReg Rd, u8 i) { return RegOp8b{ TI(i), Rd, CMP }.v; }
	static inline TI add(LoReg Rd, u8 i) { return RegOp8b{ TI(i), Rd, ADD }.v; }
	static inline TI sub(LoReg Rd, u8 i) { return RegOp8b{ TI(i), Rd, SUB }.v; }

	static inline TI ldrPc(LoReg Rd, Value10 offset) { return RegOp8b{ TI(offset >> 2), Rd, LDR_PC }.v;}

	static inline TI strSp(LoReg Rd, Value10 offset) { return RegOp8b{ TI(offset >> 2), Rd, STR_SP }.v; }
	static inline TI ldrSp(LoReg Rd, Value10 offset) { return RegOp8b{ TI(offset >> 2), Rd, LDR_SP }.v; }
	static inline TI addPc(LoReg Rd, Value10 offset) { return RegOp8b{ TI(offset >> 2), Rd, ADD_PC }.v; }
	static inline TI addSp(LoReg Rd, Value10 offset) { return RegOp8b{ TI(offset >> 2), Rd, ADD_SP }.v; }

	//RegOp11b

	/*static inline TI b(Value12 offset) { return RegOp11b{ TI(offset >> 1), B }.v;}
	static inline TI bl(Value23 offset, bool high) { return RegOp11b{ TI(offset >> (12_usz * high + 1)), BLL - high }.v;}*/

	//RegOp12b

	static inline TI b(Condition cond, i8 i) { return RegOp12b{ u8(i), cond, B0 >> 1 }.v; }
	static inline TI swi(u8 op) { return RegOp12b{ op, 0b1111, B0 >> 1 }.v; }

}