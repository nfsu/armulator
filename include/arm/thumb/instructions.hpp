#pragma once
#include "reg_op.hpp"
#include "../condition.hpp"
#include "types/bitset.hpp"

namespace arm::thumb {

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
	static inline TI bx(LoReg Rs) { return RegOp0b{ r0, Rs, BX_LO }.v; }
	static inline TI bx(HiReg Hs) { return RegOp0b{ r0, Hs, BX_HI }.v; }

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

	static inline TI addSp(i16 value /* -/+ 508 */) { 
		return RegOp8b{ TI((u8(oic::Math::abs(value) >> 2) & ~0x80) | (value < 0 ? 0x80 : 0)), 0, INCR_SP }.v;
	}

	//RegOp11b

	/*static inline TI b(Value12 offset) { return RegOp11b{ TI(offset >> 1), B }.v;}*/

	static inline TI bll(i32 offset /* 23-bit */) { return TI((BLL << 11) | ((u32(offset) & 0x000FFE) >> 1)); }
	static inline TI blh(i32 offset /* 23-bit */) { return TI((BLH << 11) | ((u32(offset) & 0x7FF000) >> 12)); }

	//RegOp12b

	static inline TI b(Condition cond, i16 i /* 9-bit */) { return RegOp12b{ u8(i8(i / 2)), cond, B0 >> 1 }.v; }
	static inline TI swi(u8 op) { return RegOp12b{ op, 0b1111, B0 >> 1 }.v; }

}