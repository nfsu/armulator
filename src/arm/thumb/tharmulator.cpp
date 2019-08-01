#include "arm/armulator.hpp"
#include "arm/thumb/instructions.hpp"
#include "arm/helper.hpp"
using namespace arm::thumb;

#ifdef __ALLOW_DEBUG__

#include <sstream>

template<typename T>
String num(const T &t) {
	std::stringstream ss;
	ss << t;
	return ss.str();
}

	#define printOp2i(name, Rd, Rs, i)																			\
		oic::System::log()->println(String(#name) + " r" + num(Rd) + ", r" + num(Rs) + ", #" + num(i));

	#define printOp1i(name, Rd, i)																				\
		oic::System::log()->println(String(#name) + " r" + num(Rd) + ", #" + num(i));

	#define printOpi(name, i)																					\
		oic::System::log()->println(String(#name) + " #" + num(i));

	#define printOp1(name, Rs)																					\
		oic::System::log()->println(String(#name) + " r" + num(Rs));

	#define printOp2(name, Rd, Rs)																				\
		oic::System::log()->println(String(#name) + " r" + num(Rd) + ", r" + num(Rs));

	#define printOp3(name, Rd, Rs, Rn)																			\
		oic::System::log()->println(String(#name) + " r" + num(Rd) + ", r" + num(Rs) + ", r" + num(Rn));

#else
	#define printOp2i(name, ...) 
	#define printOp1i(name, ...) 
	#define printOpi(name, ...) 
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
#define Op9_7 (r.ir >> 7)
#define Op10_6 (r.ir >> 6)
#define i7_0 (r.ir & 0x7F)
#define i8_0 (r.ir & 0xFF)
#define Rd3_8 ((r.ir >> 8) & 7)
#define Op8_8 (r.ir >> 8)

//Fill next instruction pipeline

template<bool isThumb>
__INLINE__ void fetchNext(arm::Registers &r, arm::Memory32 &memory) {

	r.ir = r.nir;

	if constexpr (isThumb) {
		r.nir = memory.get<u16>(r.pc);
		r.pc += 2;
	} else {
		r.nir = memory.get<u32>(r.pc);
		r.pc += 4;
	}

}

//Step through a thumb instruction
//Where m is the mapping of high registers (type mapping + 8) so m[HiReg] matches the register id it should fetch from
//Normal instructions take 1 cycle

template<arm::Armulator::Version v, bool ascendingStack = false, bool emptyStack = false>
__INLINE__ void stepThumb(arm::Registers &r, arm::Memory32 &memory, const u8 *&m, u64 &cycles) {

	using Stack = arm::Stack32<ascendingStack, emptyStack>;

	u32 reg;

	switch (Op5_11 /* fetch first 5 bits of opcode */) {

		//Layout:
		//Rd3_0, Rs3_3, i5_6, Op5_11

		case LSL:		//Logical shift left
			printOp2i(LSL, Rd3_0, Rs3_3, i5_6);
			reg = r.loReg[Rd3_0] = r.loReg[Rs3_3] << i5_6;
			goto setNZC;

		case LSR:		//Logical shift right
			printOp2i(LSR, Rd3_0, Rs3_3, i5_6);
			reg = r.loReg[Rd3_0] = r.loReg[Rs3_3] >> i5_6;
			goto setNZC;

		case ASR:		//Arithmetic shift right (maintain sign)
			printOp2i(ASR, Rd3_0, Rs3_3, i5_6);
			reg = r.loReg[Rd3_0] = arm::asr(r.loReg[Rs3_3], i5_6);
			goto setNZC;

		case STRi:		//Store u32 (with intermediate offset)
			printOp2i(STR, Rd3_0, Rs3_3, i5_6_2);
			++cycles;
			memory.set(r.loReg[Rs3_3] + i5_6_2, u32(r.loReg[Rd3_0]));
			goto fetch;

		case LDRi:		//Load u32 (with intermediate offset)
			printOp2i(LDR, Rd3_0, Rs3_3, i5_6_2);
			cycles += 2;
			r.loReg[Rd3_0] = memory.get<u32>(r.loReg[Rs3_3] + i5_6_2);
			goto fetch;

		case STRBi:		//Store u8 (with intermediate offset)
			printOp2i(STRB, Rd3_0, Rs3_3, i5_6);
			++cycles;
			memory.set(r.loReg[Rs3_3] + i5_6, u8(r.loReg[Rd3_0]));
			goto fetch;

		case LDRBi:		//Store u8 (with intermediate offset)
			printOp2i(LDRB, Rd3_0, Rs3_3, i5_6);
			cycles += 2;
			r.loReg[Rd3_0] = memory.get<u8>(r.loReg[Rs3_3] + i5_6);
			goto fetch;

		case STRHi:		//Store u16 (with intermediate offset)
			printOp2i(STRH, Rd3_0, Rs3_3, i5_6_1);
			++cycles;
			memory.set(r.loReg[Rs3_3] + i5_6_1, u16(r.loReg[Rd3_0]));
			goto fetch;

		case LDRHi:		//Store u16 (with intermediate offset)
			printOp2i(LDRH, Rd3_0, Rs3_3, i5_6_1);
			cycles += 2;
			r.loReg[Rd3_0] = memory.get<u16>(r.loReg[Rs3_3] + i5_6_1);
			goto fetch;

		//Layout:
		//i8_0, Rd3_8, Op5_11

		case MOV:
			printOp1i(MOV, Rd3_8, i8_0);
			reg = r.loReg[Rd3_8] = i8_0;
			goto setNZ;

		case CMP:
			printOp1i(CMP, Rd3_8, i8_0);
			reg = r.loReg[Rd3_8] - i8_0;
			goto setNZCV;

		case ADD:
			printOp1i(ADD, Rd3_8, i8_0);
			reg = r.loReg[Rd3_8] += i8_0;
			goto setNZCV;

		case SUB:
			printOp1i(SUB, Rd3_8, i8_0);
			reg = r.loReg[Rd3_8] -= i8_0;
			goto setNZCV;

		case STR_SP:
			printOp2i(STR, Rd3_8, u32(arm::sp), i8_0 << 2);
			memory.set(r.registers[m[HiReg::sp]] + (i8_0 << 2), r.loReg[Rd3_8]);
			goto fetch;

		case LDR_SP:
			printOp2i(STR, Rd3_8, u32(arm::sp), i8_0 << 2);
			r.loReg[Rd3_8] = memory.get<u32>(r.registers[m[HiReg::sp]] + (i8_0 << 2));
			goto fetch;

		case LDR_PC:
			printOp2i(LDR, Rd3_8, u32(arm::pc), i8_0 << 2);
			r.loReg[Rd3_8] = memory.get<u32>(((r.pc >> 2) + i8_0) << 2);
			goto fetch;

		case ADD_PC:
			printOp2i(ADD, Rd3_8, u32(arm::pc), i8_0 << 2);
			r.loReg[Rd3_8] = r.pc + (i8_0 << 2);
			goto fetch;

		case ADD_SP:
			printOp2i(ADD, Rd3_8, u32(arm::sp), i8_0 << 2);
			r.loReg[Rd3_8] = r.registers[m[HiReg::sp]] + (i8_0 << 2);
			goto fetch;


		//Load/store multiple
		//LDMIA takes 2 + n cycles
		//STMIA takes 1 + n cycles

		case STMIA:

			//TODO: Opcode?

			for (usz i = 0; i < 8; ++i)
				if (r.ir & (1 << i)) {
					memory.set(r.loReg[Rd3_8], r.loReg[i]);
					r.loReg[Rd3_8] += 4;
					++cycles;
				}

			goto fetch;

		case LDMIA:

			++cycles;

			//TODO: Opcode?

			for (usz i = 0; i < 8; ++i)
				if (r.ir & (0x80 >> i)) {
					memory.set(r.loReg[Rd3_8], r.loReg[7 - i]);
					r.loReg[Rd3_8] -= 4;
					++cycles;
				}

			goto fetch;

		//Unconditional (thumb) branch
		//Interpret 11-bit 2's complement as u32
		//Assembler takes into account prefetch
		//Takes 3 cycles

		case B:
			reg = ((r.ir << 1) & 0xFFE) | (r.ir & 0x400 * (0xFFFFF000 / 0x400));
			printOpi(B, i32(reg));
			r.pc += reg;
			goto branchThumb;

		//Conditional (thumb) branch
		//Assembler takes into account prefetch
		//Takes 3 cycles

		case B0:
		case B1:

			switch (Op8_8) {

				case BEQ:

					printOpi(BEQ, i32(i8(i8_0)) * 2);

					if (!r.cpsr.zero())
						goto fetch;

					break;

				case BNE:

					printOpi(BNE, i32(i8(i8_0)) * 2);

					if (r.cpsr.zero())
						goto fetch;

					break;

				case BCS:

					printOpi(BCS, i32(i8(i8_0)) * 2);

					if (!r.cpsr.carry())
						goto fetch;

					break;

				case BCC:

					printOpi(BCC, i32(i8(i8_0)) * 2);

					if (r.cpsr.carry())
						goto fetch;

					break;

				case BMI:

					printOpi(BMI, i32(i8(i8_0)) * 2);

					if (!r.cpsr.negative())
						goto fetch;

					break;

				case BPL:

					printOpi(BPL, i32(i8(i8_0)) * 2);

					if (r.cpsr.negative())
						goto fetch;

					break;

				case BVS:

					printOpi(BVS, i32(i8(i8_0)) * 2);

					if (!r.cpsr.overflow())
						goto fetch;

					break;

				case BVC:

					printOpi(BVC, i32(i8(i8_0)) * 2);

					if (r.cpsr.overflow())
						goto fetch;

					break;

				case BHI:

					printOpi(BHI, i32(i8(i8_0)) * 2);

					if (!r.cpsr.carry() || r.cpsr.zero())
						goto fetch;

					break;

				case BLS:

					printOpi(BLS, i32(i8(i8_0)) * 2);

					if (r.cpsr.carry() && !r.cpsr.zero())
						goto fetch;

					break;

				case BGE:

					printOpi(BGE, i32(i8(i8_0)) * 2);

					if (r.cpsr.negative() != r.cpsr.overflow())
						goto fetch;

					break;

				case BLT:

					printOpi(BLT, i32(i8(i8_0)) * 2);

					if (r.cpsr.negative() == r.cpsr.overflow())
						goto fetch;

					break;

				case BGT:

					printOpi(BGT, i32(i8(i8_0)) * 2);

					if (r.cpsr.zero() || r.cpsr.negative() != r.cpsr.overflow())
						goto fetch;

					break;

				case BLE:

					printOpi(BLE, i32(i8(i8_0)) * 2);

					if (!r.cpsr.zero() && r.cpsr.negative() == r.cpsr.overflow())
						goto fetch;

					break;

				case BAL:

					printOpi(BAL, i32(i8(i8_0)) * 2);
					break;

				case SWI:

					printOpi(SWI, u32(i8_0));
					r.exception<arm::Exception::SWI>();
					goto branchArm;
			}

			r.pc += u32(i8(i8_0)) << 1;
			goto branchThumb;

		//Long (thumb) branch with link
		//Fetch nir and construct 23-bit two's complement and jump to it
		//Assembler takes into account prefetch
		//Takes 4 cycles
		case BLL:

			++cycles;

			reg = (((r.ir << 1) & 0xFFE) | ((r.nir << 12) & 0x7FF000)) | (r.nir & 0x400 * (0xFF800000 / 0x400));

			printOpi(BL, i32(reg));
			r.registers[m[HiReg::lr]] = (r.pc - 2) | 1;		//Next instruction into LR
			r.pc += reg;

			goto branchThumb;

		//Long branch with link
		//Fetch nir and construct 23-bit two's complement and jump to it
		//Assembler takes into account prefetch
		//Takes 4 cycles
		case BLX:

			if constexpr ((v & 0xFF) >= arm::Armulator::VersionSpec::v5) {

				++cycles;

				reg = (((r.ir << 1) & 0xFFE) | ((r.nir << 12) & 0x7FF000)) | (r.nir & 0x400 * (0xFF800000 / 0x400));

				printOpi(BLX, i32(reg));
				r.registers[m[HiReg::lr]] = (r.pc - 2) | 1;		//Next instruction into LR
				r.pc += reg;

				r.cpsr.clearThumb();
				goto branchArm;
			}
			else
				goto undef;

		//Layout:
		//Rd3_0, Rs3_3, Rni3_6, Op7_9

		case ADD_SUB:
		case ST:
		case LD:

			switch (Op7_9) {

				case ADD_R:
					printOp3(ADD, Rd3_0, Rs3_3, Rni3_6);
					reg = r.loReg[Rd3_0] = r.loReg[Rs3_3] + r.loReg[Rni3_6];
					goto setNZCV;

				case SUB_R:
					printOp3(ADD, Rd3_0, Rs3_3, Rni3_6);
					reg = r.loReg[Rd3_0] = r.loReg[Rs3_3] - r.loReg[Rni3_6];
					goto setNZCV;

				case ADD_3B:
					printOp2i(ADD, Rd3_0, Rs3_3, Rni3_6);
					reg = r.loReg[Rd3_0] = r.loReg[Rs3_3] + Rni3_6;
					goto setNZCV;

				case SUB_3B:
					printOp2i(ADD, Rd3_0, Rs3_3, Rni3_6);
					reg = r.loReg[Rd3_0] = r.loReg[Rs3_3] - Rni3_6;
					goto setNZCV;

				case STR:
					printOp3(STR, Rd3_0, Rs3_3, Rni3_6);
					memory.set(r.loReg[Rs3_3] + r.loReg[Rni3_6], r.loReg[Rd3_0]);
					goto fetch;

				case STRH:
					printOp3(STRH, Rd3_0, Rs3_3, Rni3_6);
					memory.set(r.loReg[Rs3_3] + r.loReg[Rni3_6], u16(r.loReg[Rd3_0]));
					goto fetch;

				case STRB:
					printOp3(STRB, Rd3_0, Rs3_3, Rni3_6);
					memory.set(r.loReg[Rs3_3] + r.loReg[Rni3_6], u8(r.loReg[Rd3_0]));
					goto fetch;

				case LDSB:
					printOp3(LDSB, Rd3_0, Rs3_3, Rni3_6);
					reg = memory.get<u8>(r.loReg[Rs3_3] + r.loReg[Rni3_6]);
					r.loReg[Rd3_0] = reg | (reg & 0x80 * (0xFFFFFF00 / 0x80));
					goto fetch;

				case LDR:
					printOp3(LDR, Rd3_0, Rs3_3, Rni3_6);
					r.loReg[Rd3_0] = memory.get<u32>(r.loReg[Rs3_3] + r.loReg[Rni3_6]);
					goto fetch;

				case LDRH:
					printOp3(LDRH, Rd3_0, Rs3_3, Rni3_6);
					r.loReg[Rd3_0] = memory.get<u16>(r.loReg[Rs3_3] + r.loReg[Rni3_6]);
					goto fetch;

				case LDRB:
					printOp3(LDRB, Rd3_0, Rs3_3, Rni3_6);
					r.loReg[Rd3_0] = memory.get<u8>(r.loReg[Rs3_3] + r.loReg[Rni3_6]);
					goto fetch;

				case LDSH:
					printOp3(LDSH, Rd3_0, Rs3_3, Rni3_6);
					reg = memory.get<u16>(r.loReg[Rs3_3] + r.loReg[Rni3_6]);
					r.loReg[Rd3_0] = reg | (reg & 0x8000 * (0xFFFF0000 / 0x8000));
					goto fetch;
			}

		//Layout:
		//Rd3_0, Rs3_3, Op10_6

		case ALU_HI_BX:

				switch (Op10_6) {

					case AND:
						printOp2(AND, Rd3_0, Rs3_3);
						reg = r.loReg[Rd3_0] &= r.loReg[Rs3_3];
						goto setNZ;

					case EOR:
						printOp2(EOR, Rd3_0, Rs3_3);
						reg = r.loReg[Rd3_0] ^= r.loReg[Rs3_3];
						goto setNZ;

					case LSL_R:
						printOp2(LSL, Rd3_0, Rs3_3);
						++cycles;
						reg = r.loReg[Rd3_0] <<= r.loReg[Rs3_3];
						goto setNZC;

					case LSR_R:
						printOp2(LSR, Rd3_0, Rs3_3);
						++cycles;
						reg = r.loReg[Rd3_0] >>= r.loReg[Rs3_3];
						goto setNZC;

					case ASR_R:
						printOp2(ASR, Rd3_0, Rs3_3);
						++cycles;
						reg = r.loReg[Rd3_0] = arm::asr(r.loReg[Rd3_0], r.loReg[Rs3_3]);
						goto setNZC;

					case ADC:
						printOp2(ADC, Rd3_0, Rs3_3);
						reg = r.loReg[Rd3_0] -= r.loReg[Rs3_3] + !r.cpsr.carry();
						goto setNZCV;

					case SBC:
						printOp2(SBC, Rd3_0, Rs3_3);
						reg = r.loReg[Rd3_0] += r.loReg[Rs3_3] + r.cpsr.carry();
						goto setNZCV;

					case ROR:
						printOp2(ROR, Rd3_0, Rs3_3);
						reg = r.loReg[Rd3_0] = arm::ror(r.loReg[Rd3_0], r.loReg[Rs3_3]);
						++cycles;
						goto setNZC;

					case TST:
						printOp2(TST, Rd3_0, Rs3_3);
						reg = r.loReg[Rd3_0] & r.loReg[Rs3_3];
						goto setNZ;

					case NEG:
						printOp2(NEG, Rd3_0, Rs3_3);
						reg = r.loReg[Rd3_0] = u32(-i32(r.loReg[Rs3_3]));
						goto setNZCV;

					case CMP_R:
						printOp2(CMP, Rd3_0, Rs3_3);
						reg = r.loReg[Rd3_0] - r.loReg[Rs3_3];
						goto setNZCV;

					case CMN:
						printOp2(CMN, Rd3_0, Rs3_3);
						reg = r.loReg[Rd3_0] + r.loReg[Rs3_3];
						goto setNZCV;

					case ORR:
						printOp2(ORR, Rd3_0, Rs3_3);
						reg = r.loReg[Rd3_0] |= r.loReg[Rs3_3];
						goto setNZ;

					//TODO: Multiply takes 1 + n cycles on ARM7
					//n = 1 if front multiplier 24 bits are 0 or 1
					//n = 2 if front multiplier 16 bits are 0 or 1
					//n = 3 if front multiplier 8 bits are 0 or 1
					//Default: n = 4, n = 3 on ARM9
					case MUL:

						printOp2(MUL, Rd3_0, Rs3_3);
						reg = r.loReg[Rd3_0] *= r.loReg[Rs3_3];
						
						if constexpr((v & 0xFF) <= arm::Armulator::VersionSpec::v4)
							r.cpsr.value &= ~r.cpsr.cMask;

						goto setNZ;

					case BIC:
						printOp2(BIC, Rd3_0, Rs3_3);
						reg = r.loReg[Rd3_0] &= ~r.loReg[Rs3_3];
						goto setNZ;

					case MVN:
						printOp2(MVN, Rd3_0, Rs3_3);
						reg = r.loReg[Rd3_0] = ~r.loReg[Rs3_3];
						goto setNZ;

					case ADD_LO_HI:
						printOp2(ADD, Rd3_0, Rs3_3 | 8);
						reg = r.loReg[Rd3_0] += r.registers[m[Rs3_3]];
						goto fetch;

					case ADD_HI_LO:

						printOp2(ADD, Rd3_0 | 8, Rs3_3);
						reg = r.registers[m[Rd3_0]] += r.loReg[Rs3_3];

						if (Rd3_0 == HiReg::r15)
							goto branch;

						goto fetch;

					case ADD_HI_HI:

						printOp2(ADD, Rd3_0 | 8, Rs3_3 | 8);
						reg = r.registers[m[Rd3_0]] += r.registers[m[Rs3_3]];

						if (Rd3_0 == HiReg::r15)
							goto branch;

						goto fetch;

					case CMP_LO_HI:
						printOp2(CMP, Rd3_0, Rs3_3 | 8);
						reg = r.loReg[Rd3_0] - r.registers[m[Rs3_3]];
						goto setNZCV;

					case CMP_HI_LO:
						printOp2(CMP, Rd3_0 | 8, Rs3_3);
						reg = r.registers[m[Rd3_0]] - r.loReg[Rs3_3];
						goto setNZCV;

					case CMP_HI_HI:
						printOp2(CMP, Rd3_0 | 8, Rs3_3 | 8);
						reg = r.registers[m[Rd3_0]] - r.registers[m[Rs3_3]];
						goto setNZCV;

					case MOV_LO_HI:
						printOp2(MOV, Rd3_0, Rs3_3 | 8);
						reg = r.loReg[Rd3_0] = r.registers[m[Rs3_3]];
						goto fetch;

					case MOV_HI_LO:

						printOp2(MOV, Rd3_0 | 8, Rs3_3);
						reg = r.registers[m[Rd3_0]] = r.loReg[Rs3_3];

						if (Rd3_0 == HiReg::pc)
							goto branch;

						goto fetch;

					case MOV_HI_HI:

						printOp2(MOV, Rd3_0 | 8, Rs3_3 | 8);
						reg = r.registers[m[Rd3_0]] = r.registers[m[Rs3_3]];

						if (Rd3_0 == HiReg::pc)
							goto branch;

						goto fetch;

					//Branch and Exchange
					//1 cycle + 2 cycle prefetch = 3 cycles

					case BX_LO:
						printOp1(BX, Rs3_3);
						r.pc = r.loReg[Rs3_3];
						goto branchExchange;

					case BX_HI:
						printOp1(BX, Rs3_3 | 8);
						r.pc = r.registers[m[Rs3_3]];
						goto branchExchange;

			}

		//Layout:
		//i7_0, Op9_7

		case INCR_SP:
		case PUSH_POP:

			switch (Op9_7) {

				case ADD_TO_SP:
					printOp1i(ADD, u32(arm::sp), i7_0 << 2);
					r.registers[m[HiReg::sp]] += i7_0 << 2;
					goto fetch;

				case SUB_TO_SP:
					printOp1i(ADD, u32(arm::sp), -i32(i7_0 << 2));
					r.registers[m[HiReg::sp]] -= i7_0 << 2;
					goto fetch;

				//Push and pop instructions
				//Assuming r0 is located closest to the top and lr is located furthest from the top

				case PUSH_LR:

					//TODO: Opcode

					Stack::push(memory, r.registers[m[HiReg::sp]], r.registers[m[HiReg::lr]] | 1);
					++cycles;

				case PUSH:

					//TODO: Opcode

					for(usz i = 0; i < 8; ++i)
						if (r.ir & (0x80 >> i)) {
							Stack::push(memory, r.registers[m[HiReg::sp]], r.loReg[7 - i]);
							++cycles;
						}

					goto fetch;

				case POP_PC:

					//TODO: Opcode

					++cycles;

				case POP:

					//TODO: Opcode

					for (usz i = 0; i < 8; ++i)
						if (r.ir & (1 << i)) {
							Stack::pop(memory, r.registers[m[HiReg::sp]], r.loReg[i]);
							++cycles;
						}

					if (r.ir & 0x100) {
						Stack::pop(memory, r.registers[m[HiReg::sp]], r.pc);
						goto branch;
					}

					goto fetch;

				case BKPT:

					if constexpr ((v & 0xFF) >= arm::Armulator::VersionSpec::v5) {
						printOpi(BKPT, r.ir & 0xFF);
						r.exception<arm::Exception::PREFETCH_ABORT>();
						goto branchArm;
					}

			}

		default:
			goto undef;

		//TODO: Aborts; prefetch and data abort
		//TODO: USR mode should enable bios protection, otherwise disable it

	}

undef:

	r.exception<arm::Exception::UND>();

	#ifdef __USE_EXIT__
		return;
	#else
		goto branchArm;
	#endif

//Called after every branch instruction that doesn't require exchange
branch:

	//<=ARM7 doesn't allow switching modes in non-exchange branches
	if constexpr ((v & 0xFF) <= arm::Armulator::VersionSpec::v4) {
		r.pc &= ~1;
		goto branchThumb;
	}

//Used for BX instruction on ARM7 and always for register PC modifications >ARM9
branchExchange:

	if (r.pc & 1) {
		r.pc &= ~1;
		goto branchThumb;
	}

	r.cpsr.clearThumb();

branchArm:

	//Fill up pipeline
	fetchNext<false>(r, memory);
	fetchNext<false>(r, memory);
	m -= 8;								//Thumb always uses the upper mappings through m[HiReg] not m[Register]
	goto endBranch;

branchThumb:

	//Fill up pipeline
	fetchNext<true>(r, memory);
	fetchNext<true>(r, memory);

endBranch:

	//Requires at least 3 cycles (2 for fetch, 1 for pipeline)

	cycles += 2;
	return;

setNZCV:

	//TODO:

setNZC:

	//TODO:

setNZ:

	r.cpsr.negative(reg & 0x80000000);
	r.cpsr.zero(reg == 0);

fetch:

	fetchNext<true>(r, memory);

}