#include "arm/armulator.hpp"
#include "arm/thumb/instructions.hpp"
#include "arm/helper.hpp"
using namespace arm::thumb;

//TODO: Temp

#include <sstream>

template<typename T>
String num(const T &t) {
	std::stringstream ss;
	ss << t;
	return ss.str();
}

#ifdef __ALLOW_DEBUG__

	#define printOp2i(name, Rd, Rs, i)																			\
		oic::System::log()->println(String(#name) + " r" + num(Rd) + ", r" + num(Rs) + ", #" + num(i));

	#define printOp1i(name, Rd, i)																				\
		oic::System::log()->println(String(#name) + " r" + num(Rd) + ", #" + num(i));

	#define printOp1(name, Rs)																					\
		oic::System::log()->println(String(#name) + " r" + num(Rs));

	#define printOp2(name, Rd, Rs)																				\
		oic::System::log()->println(String(#name) + " r" + num(Rd) + ", r" + num(Rs));

	#define printOp3(name, Rd, Rs, Rn)																			\
		oic::System::log()->println(String(#name) + " r" + num(Rd) + ", r" + num(Rs) + ", r" + num(Rn));

#else
	#define printOp2i(name, ...) 
	#define printOp1i(name, ...) 
	#define printOp1(name, ...) 
	#define printOp2(name, ...) 
	#define printOp3(name, ...) 
#endif

//

//Defines are as follows:

//Rd/Rs/i/Op = Type;
//s = source, d = destination, n = intermediate
//i = intermediate, Op = opcode, ni = intermediate or register (depends on opcode)

//size_offset in bits
//_lshift is for when a left shift is applied to the data after it is obtained

//Rd3_0 = Low destination 3-bit register at bit 0 offset with no lshift
//i5_6_2 = 5-bit intermediate at bit 6 with 2 lshift

#define Rd3_0 (r.ir & 7)
#define Rs3_3 ((r.ir >> 3) & 7)
#define Rni3_6 ((r.ir >> 6) & 7)
#define i5_6 ((r.ir >> 6) & 0x1F)
#define i5_6_1 ((r.ir >> 5) & 0x3E)
#define i5_6_2 ((r.ir >> 4) & 0x7C)
#define Op5_11 (r.ir >> 11)
#define Op7_9 (r.ir >> 9)
#define Op10_6 (r.ir >> 6)
#define i8_0 (r.ir & 0xFF)
#define Rd3_8 ((r.ir >> 8) & 7)

//Step through an arm instruction
//Where m is the mapping of high registers (type mapping + 8) so m[HiReg] matches the register id it should fetch from
//Normal instructions take 1 cycle

//TODO: load multiple takes 2 + n cycles
//TODO: store multiple takes 1 + n cycles

__INLINE__ u32 stepThumb(arm::Registers &r, arm::Memory32 &memory, const u8 *&m, bool &condition) {

	switch (Op5_11 /* fetch first 5 bits of opcode */) {

		//Layout:
		//Rd3_0, Rs3_3, i5_6, Op5_11
		//TODO: Shifts take 2 cycles

		case LSL:		//Logical shift left
			printOp2i(LSL, Rd3_0, Rs3_3, i5_6);
			return r.loReg[Rd3_0] = r.loReg[Rs3_3] << i5_6;

		case LSR:		//Logical shift right
			printOp2i(LSR, Rd3_0, Rs3_3, i5_6);
			return r.loReg[Rd3_0] = r.loReg[Rs3_3] >> i5_6;

		case ASR:		//Arithmetic shift right (maintain sign)
			printOp2i(ASR, Rd3_0, Rs3_3, i5_6);
			return r.loReg[Rd3_0] = arm::asr(r.loReg[Rs3_3], i5_6);

		//TODO: STR = 2 cycles, LDR = 3 cycles

		case STRi:		//Store u32 (with intermediate offset)
			printOp2i(STR, Rd3_0, Rs3_3, i5_6_2);
			condition = false;
			return memory.set(r.loReg[Rs3_3] + i5_6_2, u32(r.loReg[Rd3_0]));

		case LDRi:		//Load u32 (with intermediate offset)
			printOp2i(LDR, Rd3_0, Rs3_3, i5_6_2);
			condition = false;
			return r.loReg[Rd3_0] = memory.get<u32>(r.loReg[Rs3_3] + i5_6_2);

		case STRBi:		//Store u8 (with intermediate offset)
			printOp2i(STRB, Rd3_0, Rs3_3, i5_6);
			condition = false;
			return memory.set(r.loReg[Rs3_3] + i5_6, u8(r.loReg[Rd3_0]));

		case LDRBi:		//Store u8 (with intermediate offset)
			printOp2i(LDRB, Rd3_0, Rs3_3, i5_6);
			condition = false;
			return r.loReg[Rd3_0] = memory.get<u8>(r.loReg[Rs3_3] + i5_6);

		case STRHi:		//Store u16 (with intermediate offset)
			printOp2i(STRH, Rd3_0, Rs3_3, i5_6_1);
			condition = false;
			return memory.set(r.loReg[Rs3_3] + i5_6_1, u16(r.loReg[Rd3_0]));

		case LDRHi:		//Store u16 (with intermediate offset)
			printOp2i(LDRH, Rd3_0, Rs3_3, i5_6_1);
			condition = false;
			return r.loReg[Rd3_0] = memory.get<u16>(r.loReg[Rs3_3] + i5_6_1);

		//Layout:
		//i8_0, Rd3_8, Op5_11

		case MOV:
			printOp1i(MOV, Rd3_8, i8_0);
			return r.loReg[Rd3_8] = i8_0;

		case CMP:
			printOp1i(CMP, Rd3_8, i8_0);
			return r.loReg[Rd3_8] - i8_0;

		case ADD:
			printOp1i(ADD, Rd3_8, i8_0);
			return r.loReg[Rd3_8] += i8_0;

		case SUB:
			printOp1i(SUB, Rd3_8, i8_0);
			return r.loReg[Rd3_8] -= i8_0;

		//Unconditional branch
		//Interpret 11-bit 2's complement as u32
		//Assembler takes into account prefetch
		//TODO: Should set "branch" condition, so it does a prefetch for next instructions

		case B:
			condition = false;
			return r.pc += ((r.nir << 1) & 0xFFE) | (0x3FFFFC * (r.nir & 0x400));

		//TODO: Fetch nir and construct 23-bit two's complement and jump to it
		//Takes 4 cycles
		//case BLH:

		//Layout:
		//Rd3_0, Rs3_3, Rni3_6, Op7_9

		case ADD_SUB:

			switch (Op7_9) {

				case ADD_R:
					printOp3(ADD, Rd3_0, Rs3_3, Rni3_6);
					return r.loReg[Rd3_0] = r.loReg[Rs3_3] + r.loReg[Rni3_6];

				case SUB_R:
					printOp3(ADD, Rd3_0, Rs3_3, Rni3_6);
					return r.loReg[Rd3_0] = r.loReg[Rs3_3] - r.loReg[Rni3_6];

				case ADD_3B:
					printOp2i(ADD, Rd3_0, Rs3_3, Rni3_6);
					return r.loReg[Rd3_0] = r.loReg[Rs3_3] + Rni3_6;

				case SUB_3B:
					printOp2i(ADD, Rd3_0, Rs3_3, Rni3_6);
					return r.loReg[Rd3_0] = r.loReg[Rs3_3] - Rni3_6;

				//TODO: STR, STRH, STRB, LDSB, LDR, LDRH, LDRB, LDSH
			}

		//Layout:
		//Rd3_0, Rs3_3, Op10_6

		case ALU_HI_BX:

				switch (Op10_6) {

					case AND:
						printOp2(AND, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] &= r.loReg[Rs3_3];

					case EOR:
						printOp2(EOR, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] ^= r.loReg[Rs3_3];

					case LSL_R:
						printOp2(LSL, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] <<= r.loReg[Rs3_3];

					case LSR_R:
						printOp2(LSR, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] >>= r.loReg[Rs3_3];

					case ASR_R:
						printOp2(ASR, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] = arm::asr(r.loReg[Rd3_0], r.loReg[Rs3_3]);

					case ADC:
						printOp2(ADC, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] -= r.loReg[Rs3_3] + !r.cpsr.carry();

					case SBC:
						printOp2(SBC, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] += r.loReg[Rs3_3] + r.cpsr.carry();

					case ROR:
						printOp2(ROR, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] = arm::ror(r.loReg[Rd3_0], r.loReg[Rs3_3]);

					case TST:
						printOp2(TST, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] & r.loReg[Rs3_3];

					case NEG:
						printOp2(NEG, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] = u32(-i32(r.loReg[Rs3_3]));

					case CMP_R:
						printOp2(CMP, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] - r.loReg[Rs3_3];

					case CMN:
						printOp2(CMN, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] + r.loReg[Rs3_3];

					case ORR:
						printOp2(ORR, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] |= r.loReg[Rs3_3];

					//TODO: Multiply takes 1 + n cycles
					//n = 1 if front multiplier 24 bits are 0 or 1
					//n = 2 if front multiplier 16 bits are 0 or 1
					//n = 3 if front multiplier 8 bits are 0 or 1
					//Default: n = 4
					case MUL:
						printOp2(MUL, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] *= r.loReg[Rs3_3];

					case BIC:
						printOp2(BIC, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] &= ~r.loReg[Rs3_3];

					case MVN:
						printOp2(MVN, Rd3_0, Rs3_3);
						return r.loReg[Rd3_0] = ~r.loReg[Rs3_3];

					case ADD_LO_HI:
						printOp2(ADD, Rd3_0, Rs3_3 | 8);
						return r.loReg[Rd3_0] += r.registers[m[Rs3_3]];

					case ADD_HI_LO:
						printOp2(ADD, Rd3_0 | 8, Rs3_3);
						return r.registers[m[Rd3_0]] += r.loReg[Rs3_3];

					case ADD_HI_HI:
						printOp2(ADD, Rd3_0 | 8, Rs3_3 | 8);
						return r.registers[m[Rd3_0]] += r.registers[m[Rs3_3]];

					case CMP_LO_HI:
						printOp2(CMP, Rd3_0, Rs3_3 | 8);
						return r.loReg[Rd3_0] - r.registers[m[Rs3_3]];

					case CMP_HI_LO:
						printOp2(CMP, Rd3_0 | 8, Rs3_3);
						return r.registers[m[Rd3_0]] - r.loReg[Rs3_3];

					case CMP_HI_HI:
						printOp2(CMP, Rd3_0 | 8, Rs3_3 | 8);
						return r.registers[m[Rd3_0]] - r.registers[m[Rs3_3]];

					case MOV_LO_HI:
						printOp2(MOV, Rd3_0, Rs3_3 | 8);
						return r.loReg[Rd3_0] = r.registers[m[Rs3_3]];

					case MOV_HI_LO:
						printOp2(MOV, Rd3_0 | 8, Rs3_3);
						return r.registers[m[Rd3_0]] = r.loReg[Rs3_3];

					case MOV_HI_HI:
						printOp2(MOV, Rd3_0 | 8, Rs3_3 | 8);
						return r.registers[m[Rd3_0]] = r.registers[m[Rs3_3]];

					//Branch and Exchange
					//1 cycle + 2 cycle prefetch = 3 cycles
					//TODO: Should set "branch" condition, so it does a prefetch for next instructions
					//TODO: also; shouldn't incr pc after
					//TODO: Should also update m and selected range

					case BX_LO:
						printOp1(BX, Rs3_3);
						condition = false;
						r.cpsr.thumb(r.loReg[Rs3_3] & 1);
						return r.pc = r.loReg[Rs3_3] & ~1;

					case BX_HI:
						printOp1(BX, Rs3_3 | 8);
						condition = false;
						r.cpsr.thumb(r.registers[m[Rs3_3]] & 1);
						return r.pc = r.registers[m[Rs3_3]] & ~1;


			}

		default:

			oic::System::log()->fatal(String("Unsupported operation at ") + num(r.pc));
			return u32_MAX;

	}
}