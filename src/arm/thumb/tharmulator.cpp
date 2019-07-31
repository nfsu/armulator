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

template<arm::Armulator::Version>
__INLINE__ void stepThumb(arm::Registers &r, arm::Memory32 &memory, const u8 *&m, u32 &reg, u64 &timer, u64 &cycles) {

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
					goto fetch;						//TODO: Implement SWI behavior

			}

			r.pc += u32(i8(i8_0)) * 2;
			goto branchThumb;

		//Fetch nir and construct 23-bit two's complement and jump to it
		//Assembler takes into account prefetch
		//Takes 4 cycles
		case BLL:

			++cycles;

			printOpi(BL, i32((((r.ir << 1) & 0xFFE) | ((r.nir << 12) & 0x7FF000)) | (0xFFFFE000 * (r.nir & 0x400))));

			r.registers[m[HiReg::lr]] = (r.pc - 2) | 1;		//Next instruction into LR
			r.pc += (((r.ir << 1) & 0xFFE) | ((r.nir << 12) & 0x7FF000)) | (0xFFFFE000 * (r.nir & 0x400));

			goto branchThumb;

		//Layout:
		//Rd3_0, Rs3_3, Rni3_6, Op7_9

		case ADD_SUB:

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

				//TODO: STR, STRH, STRB, LDSB, LDR, LDRH, LDRB, LDSH
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
						r.cpsr.value &= ~r.cpsr.cMask;				//TODO: ARM9 Doesn't destroy carry flag
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
							goto branchThumb;

						goto fetch;

					case ADD_HI_HI:

						printOp2(ADD, Rd3_0 | 8, Rs3_3 | 8);
						reg = r.registers[m[Rd3_0]] += r.registers[m[Rs3_3]];

						if (Rd3_0 == HiReg::r15)
							goto branchThumb;

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
							goto branchThumb;

						goto fetch;

					case MOV_HI_HI:

						printOp2(MOV, Rd3_0 | 8, Rs3_3 | 8);
						reg = r.registers[m[Rd3_0]] = r.registers[m[Rs3_3]];

						if (Rd3_0 == HiReg::pc)
							goto branchThumb;

						goto fetch;

					//Branch and Exchange
					//1 cycle + 2 cycle prefetch = 3 cycles

					case BX_LO:

						printOp1(BX, Rs3_3);
						r.pc = r.loReg[Rs3_3] & ~1;

						if (r.loReg[Rs3_3] & 1)
							goto branchThumb;

						r.cpsr.value &= ~arm::PSR::tMask;
						goto branchArm;

					case BX_HI:

						printOp1(BX, Rs3_3 | 8);
						r.pc = r.registers[m[Rs3_3]] & ~1;

						if (r.loReg[Rs3_3] & 1)
							goto branchThumb;

						r.cpsr.value &= ~arm::PSR::tMask;
						goto branchArm;


			}

		//Layout:
		//Rd3_8, i8_0

		case LDR_PC:
			printOp2i(LDR, Rd3_8, u32(arm::pc), i8_0 << 2);
			r.loReg[Rd3_8] = memory.get<u32>(((r.pc >> 2) | i8_0) << 2);
			goto fetch;

		case ADD_PC:
			printOp2i(ADD, Rd3_8, u32(arm::pc), i8_0 << 2);
			r.loReg[Rd3_8] = r.pc + (i8_0 << 2);
			goto fetch;

		case ADD_SP:
			printOp2i(ADD, Rd3_8, u32(arm::sp), i8_0 << 2);
			r.loReg[Rd3_8] = r.registers[m[HiReg::sp]] + (i8_0 << 2);
			goto fetch;

		case INCR_SP:
			printOp1i(ADD, u32(arm::sp), r.ir & 0x80 ? -i32(i7_0 << 2) : i32(i7_0 << 2));
			r.registers[m[HiReg::sp]] = r.ir & 0x80 ? -i32(i7_0 << 2) : i32(i7_0 << 2);
			goto fetch;

		//TODO: STR_SP, LDR_SP

		//TODO: PUSH { Rs... }	/ PUSH { Rs..., LR }
		//TODO: POP { Rs... } / POP { Rs..., PC }

		//TODO: LDMIA takes 2 + n cycles
		//TODO: STMIA takes 1 + n cycles

		//TODO: NOP

		//TODO: ARM9:
		//TODO: BKPT (Software breakpoint) NOP
		//TODO: BLX (Link branch exchange) just checks bit 0 of register
		//TODO: PLD (Prepare cache for memory load)
		//TODO: STRD/LDRD (64-bit; two registers)
		//TODO: V5 flag; ignores extra instructions if not set. Bit 0 of POP, LD/LDM should set thumb if set.
		//TODO: Coprocessor and ABORT_BU

		default:

			timer = std::chrono::high_resolution_clock::now().time_since_epoch().count() - timer;

			oic::System::log()->warn(
				String(	"Unsupported operation at ") + num(r.pc) +
				" with time " + num(timer) + "ns (" +
				num(f64(timer) / cycles) + "ns avg, " + num(cycles) + " cycles): " + num(r.ir)
			);

			throw std::exception();
			return;

	}

branchArm:

	//Fill up pipeline
	fetchNext<false>(r, memory);
	fetchNext<false>(r, memory);
	m -= 8;								//Thumb always uses the upper mappings through m[HiReg] not m[Register]
	goto branch;

branchThumb:

	//Fill up pipeline
	fetchNext<true>(r, memory);
	fetchNext<true>(r, memory);

branch:

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