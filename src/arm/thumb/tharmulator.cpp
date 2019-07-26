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
#define Op8_8 (r.ir >> 8)

//Fill next instruction pipeline

template<bool isThumb>
__INLINE__ void fetchNext(arm::Registers &r, arm::Memory32 &memory) {

	r.ir = r.nir;

	if constexpr (isThumb) {
		r.nir = *( u16 *) memory.selected->map(r.pc);
		r.pc += 2;
	} else {
		r.nir = *( u32 *) memory.selected->map(r.pc);
		r.pc += 4;
	}

}

//Step through an arm instruction
//Where m is the mapping of high registers (type mapping + 8) so m[HiReg] matches the register id it should fetch from
//Normal instructions take 1 cycle

__INLINE__ u32 stepThumb(arm::Registers &r, arm::Memory32 &memory, const u8 *&m, bool &condition, u64 &timer, u64 &cycles) {

	switch (Op5_11 /* fetch first 5 bits of opcode */) {

		//Layout:
		//Rd3_0, Rs3_3, i5_6, Op5_11

		case LSL:		//Logical shift left
			printOp2i(LSL, Rd3_0, Rs3_3, i5_6);
			++cycles;
			return r.loReg[Rd3_0] = r.loReg[Rs3_3] << i5_6;

		case LSR:		//Logical shift right
			printOp2i(LSR, Rd3_0, Rs3_3, i5_6);
			++cycles;
			return r.loReg[Rd3_0] = r.loReg[Rs3_3] >> i5_6;

		case ASR:		//Arithmetic shift right (maintain sign)
			printOp2i(ASR, Rd3_0, Rs3_3, i5_6);
			++cycles;
			return r.loReg[Rd3_0] = arm::asr(r.loReg[Rs3_3], i5_6);

		case STRi:		//Store u32 (with intermediate offset)
			printOp2i(STR, Rd3_0, Rs3_3, i5_6_2);
			condition = false;
			++cycles;
			memory.set(r.loReg[Rs3_3] + i5_6_2, u32(r.loReg[Rd3_0]));
			return 1;

		case LDRi:		//Load u32 (with intermediate offset)
			printOp2i(LDR, Rd3_0, Rs3_3, i5_6_2);
			condition = false;
			cycles += 2;
			r.loReg[Rd3_0] = memory.get<u32>(r.loReg[Rs3_3] + i5_6_2);
			return 1;

		case STRBi:		//Store u8 (with intermediate offset)
			printOp2i(STRB, Rd3_0, Rs3_3, i5_6);
			condition = false;
			++cycles;
			memory.set(r.loReg[Rs3_3] + i5_6, u8(r.loReg[Rd3_0]));
			return 1;

		case LDRBi:		//Store u8 (with intermediate offset)
			printOp2i(LDRB, Rd3_0, Rs3_3, i5_6);
			condition = false;
			cycles += 2;
			r.loReg[Rd3_0] = memory.get<u8>(r.loReg[Rs3_3] + i5_6);
			return 1;

		case STRHi:		//Store u16 (with intermediate offset)
			printOp2i(STRH, Rd3_0, Rs3_3, i5_6_1);
			condition = false;
			++cycles;
			memory.set(r.loReg[Rs3_3] + i5_6_1, u16(r.loReg[Rd3_0]));
			return 1;

		case LDRHi:		//Store u16 (with intermediate offset)
			printOp2i(LDRH, Rd3_0, Rs3_3, i5_6_1);
			condition = false;
			cycles += 2;
			r.loReg[Rd3_0] = memory.get<u16>(r.loReg[Rs3_3] + i5_6_1);
			return 1;

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
		//Takes 3 cycles

		case B:
			printOpi(B, ((r.ir << 1) & 0xFFE) | (0x3FFFFC * (r.ir & 0x400)));
			r.pc += ((r.ir << 1) & 0xFFE) | (0x3FFFFC * (r.ir & 0x400));
			goto branchThumb;

		//Conditional branch
		//Assembler takes into account prefetch
		//Takes 3 cycles

		case B0:
		case B1:

			switch (Op8_8) {

				case BEQ:

					printOpi(BEQ, i32(i8(i8_0)) * 2);

					if (!r.cpsr.zero())
						return 0;

					break;

				case BNE:

					printOpi(BNE, i32(i8(i8_0)) * 2);

					if (r.cpsr.zero())
						return 0;

					break;

				case BCS:

					printOpi(BCS, i32(i8(i8_0)) * 2);

					if (!r.cpsr.carry())
						return 0;

					break;

				case BCC:

					printOpi(BCC, i32(i8(i8_0)) * 2);

					if (r.cpsr.carry())
						return 0;

					break;

				case BMI:

					printOpi(BMI, i32(i8(i8_0)) * 2);

					if (!r.cpsr.negative())
						return 0;

					break;

				case BPL:

					printOpi(BPL, i32(i8(i8_0)) * 2);

					if (r.cpsr.negative())
						return 0;

					break;

				case BVS:

					printOpi(BVS, i32(i8(i8_0)) * 2);

					if (!r.cpsr.overflow())
						return 0;

					break;

				case BVC:

					printOpi(BVC, i32(i8(i8_0)) * 2);

					if (r.cpsr.overflow())
						return 0;

					break;

				case BHI:

					printOpi(BHI, i32(i8(i8_0)) * 2);

					if (!r.cpsr.carry() || r.cpsr.zero())
						return 0;

					break;

				case BLS:

					printOpi(BLS, i32(i8(i8_0)) * 2);

					if (r.cpsr.carry() && !r.cpsr.zero())
						return 0;

					break;

				case BGE:

					printOpi(BGE, i32(i8(i8_0)) * 2);

					if (r.cpsr.negative() != r.cpsr.overflow())
						return 0;

					break;

				case BLT:

					printOpi(BLT, i32(i8(i8_0)) * 2);

					if (r.cpsr.negative() == r.cpsr.overflow())
						return 0;

					break;

				case BGT:

					printOpi(BGT, i32(i8(i8_0)) * 2);

					if (r.cpsr.zero() || r.cpsr.negative() != r.cpsr.overflow())
						return 0;

					break;

				case BLE:

					printOpi(BLE, i32(i8(i8_0)) * 2);

					if (!r.cpsr.zero() && r.cpsr.negative() == r.cpsr.overflow())
						return 0;

					break;

				case BAL:

					printOpi(BAL, i32(i8(i8_0)) * 2);
					break;

				case SWI:

					printOpi(SWI, u32(i8_0));
					return 0;						//TODO: Implement SWI behavior

			}

			r.pc += u32(i8(i8_0)) * 2;
			goto branchThumb;

		//Fetch nir and construct 23-bit two's complement and jump to it
		//Assembler takes into account prefetch
		//Takes 4 cycles
		case BLL:

			++cycles;

			printOpi(BL, i32((((r.ir << 1) & 0xFFE) | ((r.nir << 12) & 0x7FF000)) | (0xFFFFE000 * (r.nir & 0x400))));

			r.registers[m[HiReg::r13]] = (r.pc - 2) | 1;		//Next instruction into LR
			r.pc += (((r.ir << 1) & 0xFFE) | ((r.nir << 12) & 0x7FF000)) | (0xFFFFE000 * (r.nir & 0x400));

			goto branchThumb;

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

					case BX_LO:

						printOp1(BX, Rs3_3);
						r.cpsr.thumb(r.loReg[Rs3_3] & 1);
						r.pc = r.loReg[Rs3_3] & ~1;

						if (r.loReg[Rs3_3] & 1) {
							r.cpsr.value |= arm::PSR::tMask;
							goto branchThumb;
						}

						r.cpsr.value &= ~arm::PSR::tMask;
						goto branchArm;

					case BX_HI:
						printOp1(BX, Rs3_3 | 8);
						r.cpsr.thumb(r.registers[m[Rs3_3]] & 1);
						r.pc = r.registers[m[Rs3_3]] & ~1;
						goto branch;


			}

		//TODO: load multiple takes 2 + n cycles
		//TODO: store multiple takes 1 + n cycles

		default:

			timer = std::chrono::high_resolution_clock::now().time_since_epoch().count() - timer;

			oic::System::log()->warn(
				String(	"Unsupported operation at ") + num(r.pc) +
				" with time " + num(timer) + "ns (" +
				num(f64(timer) / cycles) + "ns avg, " + num(cycles) + " cycles)"
			);

			throw std::exception();
			return u32_MAX;

	}

branchArm:

	//Jumped to different memory range, so switch it
	memory.selected = &memory.mapRange(r.pc);

	//Fill up pipeline
	fetchNext<false>(r, memory);
	fetchNext<false>(r, memory);
	m -= 8;								//Thumb always uses the upper mappings through m[HiReg] not m[Register]
	goto branch;

branchThumb:

	//Jumped to different memory range, so switch it
	memory.selected = &memory.mapRange(r.pc);

	//Fill up pipeline
	fetchNext<true>(r, memory);
	fetchNext<true>(r, memory);

branch:

	//Don't set condition codes
	condition = false;

	//Requires at least 3 cycles (2 for fetch, 1 for pipeline)
	cycles += 2;

	return 0;

}