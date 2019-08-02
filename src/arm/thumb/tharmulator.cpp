#pragma once
#include "arm/armulator.hpp"
#include "arm/thumb/instructions.hpp"
#include "arm/helper.hpp"
#include "arm/thumb/values.hpp"
#include "arm/thumb/debug.hpp"
using namespace arm::thumb;

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

	u64 reg;
	u32 left, right, reg32;

	switch (Op5_11 /* fetch first 5 bits of opcode */) {

		//Layout:
		//Rd3_0, Rs3_3, i5_6, Op5_11

		case LSL:		//Logical shift left

			if (i5_6)
				r.cpsr.carry(r.loReg[Rs3_3] & (0x80000000 >> (i5_6 - 1)));

			reg = r.loReg[Rd3_0] = r.loReg[Rs3_3] << i5_6;
			goto setNZ;

		case LSR:		//Logical shift right

			if (i5_6)
				r.cpsr.carry(r.loReg[Rs3_3] & (1 << (i5_6 - 1)));

			reg = r.loReg[Rd3_0] = r.loReg[Rs3_3] >> i5_6;
			goto setNZ;

		case ASR:		//Arithmetic shift right (maintain sign)

			if (i5_6)
				r.cpsr.carry(r.loReg[Rd3_0] & (1 << (r.loReg[Rs3_3] - 1)));

			reg = r.loReg[Rd3_0] = arm::asr(r.loReg[Rs3_3], i5_6);
			goto setNZ;

		case STRi:		//Store u32 (with intermediate offset)
			++cycles;
			memory.set(r.loReg[Rs3_3] + i5_6_2, u32(r.loReg[Rd3_0]));
			goto fetch;

		case LDRi:		//Load u32 (with intermediate offset)
			cycles += 2;
			r.loReg[Rd3_0] = memory.get<u32>(r.loReg[Rs3_3] + i5_6_2);
			goto fetch;

		case STRBi:		//Store u8 (with intermediate offset)
			++cycles;
			memory.set(r.loReg[Rs3_3] + i5_6, u8(r.loReg[Rd3_0]));
			goto fetch;

		case LDRBi:		//Store u8 (with intermediate offset)
			cycles += 2;
			r.loReg[Rd3_0] = memory.get<u8>(r.loReg[Rs3_3] + i5_6);
			goto fetch;

		case STRHi:		//Store u16 (with intermediate offset)
			++cycles;
			memory.set(r.loReg[Rs3_3] + i5_6_1, u16(r.loReg[Rd3_0]));
			goto fetch;

		case LDRHi:		//Store u16 (with intermediate offset)
			cycles += 2;
			r.loReg[Rd3_0] = memory.get<u16>(r.loReg[Rs3_3] + i5_6_1);
			goto fetch;

		//Layout:
		//i8_0, Rd3_8, Op5_11

		case MOV:
			reg = r.loReg[Rd3_8] = i8_0;
			goto setNZ;

		case CMP:
			reg = u64(left = r.loReg[Rd3_8]) - (right = i8_0);
			goto setNZCV;

		case ADD:
			reg = u64(left = r.loReg[Rd3_8]) + (right = i8_0);
			r.loReg[Rd3_8] = u32(reg);
			goto setNZCV;

		case SUB:
			reg = u64(left = r.loReg[Rd3_8]) - (right = i8_0);
			r.loReg[Rd3_8] = u32(reg);
			goto setNZCV;

		case STR_SP:
			memory.set(r.registers[m[HiReg::sp]] + (i8_0 << 2), r.loReg[Rd3_8]);
			goto fetch;

		case LDR_SP:
			r.loReg[Rd3_8] = memory.get<u32>(r.registers[m[HiReg::sp]] + (i8_0 << 2));
			goto fetch;

		case LDR_PC:
			r.loReg[Rd3_8] = memory.get<u32>(((r.pc >> 2) + i8_0) << 2);
			goto fetch;

		case ADD_PC:
			r.loReg[Rd3_8] = r.pc + (i8_0 << 2);
			goto fetch;

		case ADD_SP:
			r.loReg[Rd3_8] = r.registers[m[HiReg::sp]] + (i8_0 << 2);
			goto fetch;

		case INCR_SP:

			reg32 = i7_0 << 2;

			if (r.ir & 0x80)
				reg32 = -i32(reg32);

			r.registers[m[HiReg::sp]] = reg32;
			goto fetch;

		//Load/store multiple
		//LDMIA takes 2 + n cycles
		//STMIA takes 1 + n cycles

		case STMIA:

			for (usz i = 0; i < 8; ++i)
				if (r.ir & (1 << i)) {
					memory.set(r.loReg[Rd3_8], r.loReg[i]);
					r.loReg[Rd3_8] += 4;
					++cycles;
				}

			goto fetch;

		case LDMIA:

			++cycles;

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
			r.pc += s12;
			goto branchThumb;

		//Conditional (thumb) branch
		//Assembler takes into account prefetch
		//Takes 3 cycles

		case B0:
		case B1:
		case PUSH_POP:

			switch (Op8_8) {

				case BEQ:

					if (!r.cpsr.zero())
						goto fetch;

					break;

				case BNE:

					if (r.cpsr.zero())
						goto fetch;

					break;

				case BCS:

					if (!r.cpsr.carry())
						goto fetch;

					break;

				case BCC:

					if (r.cpsr.carry())
						goto fetch;

					break;

				case BMI:

					if (!r.cpsr.negative())
						goto fetch;

					break;

				case BPL:

					if (r.cpsr.negative())
						goto fetch;

					break;

				case BVS:

					if (!r.cpsr.overflow())
						goto fetch;

					break;

				case BVC:

					if (r.cpsr.overflow())
						goto fetch;

					break;

				case BHI:

					if (!r.cpsr.carry() || r.cpsr.zero())
						goto fetch;

					break;

				case BLS:

					if (r.cpsr.carry() && !r.cpsr.zero())
						goto fetch;

					break;

				case BGE:

					if (r.cpsr.negative() != r.cpsr.overflow())
						goto fetch;

					break;

				case BLT:

					if (r.cpsr.negative() == r.cpsr.overflow())
						goto fetch;

					break;

				case BGT:

					if (r.cpsr.zero() || r.cpsr.negative() != r.cpsr.overflow())
						goto fetch;

					break;

				case BLE:

					if (!r.cpsr.zero() && r.cpsr.negative() == r.cpsr.overflow())
						goto fetch;

					break;

				case BAL:
					break;

				case SWI:
					r.exception<arm::Exception::SWI>();
					goto branchArm;

				//Push and pop instructions
				//Assuming r0 is located closest to the top and lr is located furthest from the top
				case PUSH_LR:

					Stack::push(memory, r.registers[m[HiReg::sp]], r.registers[m[HiReg::lr]] | 1);
					++cycles;

				case PUSH:

					for (usz i = 0; i < 8; ++i)
						if (r.ir & (0x80 >> i)) {
							Stack::push(memory, r.registers[m[HiReg::sp]], r.loReg[7 - i]);
							++cycles;
						}

					goto fetch;

				case POP_PC:
					++cycles;

				case POP:

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
						r.exception<arm::Exception::PREFETCH_ABORT>();
						goto branchArm;
					}
				
				default:
					goto undef;
			}

			r.pc += u32(i8(i8_0)) << 1;
			goto branchThumb;

		//Long (thumb) branch with link
		//Fetch nir and construct 23-bit two's complement and jump to it
		//Assembler takes into account prefetch
		//Takes 4 cycles
		case BLL:
			++cycles;
			r.registers[m[HiReg::lr]] = (r.pc - 2) | 1;		//Next instruction into LR
			r.pc += s23;
			goto branchThumb;

		//Long branch with link
		//Fetch nir and construct 23-bit two's complement and jump to it
		//Assembler takes into account prefetch
		//Takes 4 cycles
		case BLX:

			if constexpr ((v & 0xFF) >= arm::Armulator::VersionSpec::v5) {

				++cycles;
				r.registers[m[HiReg::lr]] = (r.pc - 2) | 1;		//Next instruction into LR
				r.pc += s23;
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
					reg = u64(left = r.loReg[Rs3_3]) + (right = r.loReg[Rni3_6]);
					r.loReg[Rd3_0] = u32(reg);
					goto setNZCV;

				case SUB_R:
					reg = u64(left = r.loReg[Rs3_3]) - (right = r.loReg[Rni3_6]);
					r.loReg[Rd3_0] = u32(reg);
					goto setNZCV;

				case ADD_3B:
					reg = u64(left = r.loReg[Rs3_3]) + (right = Rni3_6);
					r.loReg[Rd3_0] = u32(reg);
					goto setNZCV;

				case SUB_3B:
					reg = u64(left = r.loReg[Rs3_3]) - (right = Rni3_6);
					r.loReg[Rd3_0] = u32(reg);
					goto setNZCV;

				case STR:
					memory.set(r.loReg[Rs3_3] + r.loReg[Rni3_6], r.loReg[Rd3_0]);
					goto fetch;

				case STRH:
					memory.set(r.loReg[Rs3_3] + r.loReg[Rni3_6], u16(r.loReg[Rd3_0]));
					goto fetch;

				case STRB:
					memory.set(r.loReg[Rs3_3] + r.loReg[Rni3_6], u8(r.loReg[Rd3_0]));
					goto fetch;

				case LDSB:
					reg32 = memory.get<u8>(r.loReg[Rs3_3] + r.loReg[Rni3_6]);
					r.loReg[Rd3_0] = reg32 | (reg32 & 0x80 * (0xFFFFFF00 / 0x80));
					goto fetch;

				case LDR:
					r.loReg[Rd3_0] = memory.get<u32>(r.loReg[Rs3_3] + r.loReg[Rni3_6]);
					goto fetch;

				case LDRH:
					r.loReg[Rd3_0] = memory.get<u16>(r.loReg[Rs3_3] + r.loReg[Rni3_6]);
					goto fetch;

				case LDRB:
					r.loReg[Rd3_0] = memory.get<u8>(r.loReg[Rs3_3] + r.loReg[Rni3_6]);
					goto fetch;

				case LDSH:
					reg32 = memory.get<u16>(r.loReg[Rs3_3] + r.loReg[Rni3_6]);
					r.loReg[Rd3_0] = reg32 | (reg32 & 0x8000 * (0xFFFF0000 / 0x8000));
					goto fetch;

				default:
					goto undef;
			}

		//Layout:
		//Rd3_0, Rs3_3, Op10_6

		case ALU_HI_BX:

				switch (Op10_6) {

					case AND:
						reg = r.loReg[Rd3_0] &= r.loReg[Rs3_3];
						goto setNZ;

					case EOR:
						reg = r.loReg[Rd3_0] ^= r.loReg[Rs3_3];
						goto setNZ;

					case LSL_R:

						++cycles;

						if (r.loReg[Rs3_3])
							r.cpsr.carry(r.loReg[Rd3_0] & (0x80000000 >> (r.loReg[Rs3_3] - 1)));

						reg = r.loReg[Rd3_0] <<= r.loReg[Rs3_3];
						goto setNZ;

					case LSR_R:

						++cycles;

						if (r.loReg[Rs3_3])
							r.cpsr.carry(r.loReg[Rd3_0] & (1 << (r.loReg[Rs3_3] - 1)));

						reg = r.loReg[Rd3_0] >>= r.loReg[Rs3_3];
						goto setNZ;

					case ASR_R:

						++cycles;

						if(r.loReg[Rs3_3])
							r.cpsr.carry(r.loReg[Rd3_0] & (1 << (r.loReg[Rs3_3] - 1)));

						reg = r.loReg[Rd3_0] = arm::asr(r.loReg[Rd3_0], r.loReg[Rs3_3]);
						goto setNZ;

					case ADC:
						reg = u64(left = r.loReg[Rd3_0]) - (right = r.loReg[Rs3_3]) + !r.cpsr.carry();
						r.loReg[Rd3_0] = u32(reg);
						goto setNZCV;

					case SBC:
						reg = u64(left = r.loReg[Rd3_0]) + (right = r.loReg[Rs3_3]) + r.cpsr.carry();
						r.loReg[Rd3_0] = u32(reg);
						goto setNZCV;

					case ROR:

						if (r.loReg[Rs3_3])
							r.cpsr.carry(r.loReg[Rd3_0] & (1 << (r.loReg[Rs3_3] - 1)));

						reg = r.loReg[Rd3_0] = arm::ror(r.loReg[Rd3_0], r.loReg[Rs3_3]);
						++cycles;
						goto setNZ;

					case TST:
						reg = r.loReg[Rd3_0] & r.loReg[Rs3_3];
						goto setNZ;

					case NEG:

						r.cpsr.value &= ~r.cpsr.vMask;				//Overflow on negate doesn't exist
						r.cpsr.carry(r.loReg[Rs3_3] == i32_MIN);	//-i32_MIN doesn't exist
						reg = r.loReg[Rd3_0] = u32(-i32(r.loReg[Rs3_3]));
						goto setNZ;

					case CMP_R:
						reg = u64(left = r.loReg[Rd3_0]) - (right = r.loReg[Rs3_3]);
						goto setNZCV;

					case CMN:
						reg = u64(left = r.loReg[Rd3_0]) + (right = r.loReg[Rs3_3]);
						goto setNZCV;

					case ORR:
						reg = r.loReg[Rd3_0] |= r.loReg[Rs3_3];
						goto setNZ;

					//Multiply takes 1 + n cycles on ARM7
					//n = 1 if front multiplier 24 bits are 0 or 1
					//n = 2 if front multiplier 16 bits are 0 or 1
					//n = 3 if front multiplier 8 bits are 0 or 1
					//Default: n = 4, n = 3 on ARM9
					case MUL:

						if constexpr ((v & 0xFF) <= arm::Armulator::VersionSpec::v4) {

							r.cpsr.value &= ~r.cpsr.cMask;

							if (oic::Math::abs(i32(r.loReg[Rd3_0])) < (1 << 8))
								++cycles;
							else if (oic::Math::abs(i32(r.loReg[Rd3_0])) < (1 << 16))
								cycles += 2;
							else if (oic::Math::abs(i32(r.loReg[Rd3_0])) < (1 << 24))
								cycles += 3;
							else
								cycles += 4;

						} else
							cycles += 3;

						reg = r.loReg[Rd3_0] *= r.loReg[Rs3_3];
						goto setNZ;

					case BIC:
						reg = r.loReg[Rd3_0] &= ~r.loReg[Rs3_3];
						goto setNZ;

					case MVN:
						reg = r.loReg[Rd3_0] = ~r.loReg[Rs3_3];
						goto setNZ;

					case ADD_LO_HI:
						reg = r.loReg[Rd3_0] += r.registers[m[Rs3_3]];
						goto fetch;

					case ADD_HI_LO:

						reg = r.registers[m[Rd3_0]] += r.loReg[Rs3_3];

						if (Rd3_0 == HiReg::r15)
							goto branch;

						goto fetch;

					case ADD_HI_HI:

						reg = r.registers[m[Rd3_0]] += r.registers[m[Rs3_3]];

						if (Rd3_0 == HiReg::r15)
							goto branch;

						goto fetch;

					case CMP_LO_HI:
						reg = u64(left = r.loReg[Rd3_0]) - (right = r.registers[m[Rs3_3]]);
						goto setNZCV;

					case CMP_HI_LO:
						reg = u64(left = r.registers[m[Rd3_0]]) - (right = r.loReg[Rs3_3]);
						goto setNZCV;

					case CMP_HI_HI:
						reg = u64(left = r.registers[m[Rd3_0]]) - (right = r.registers[m[Rs3_3]]);
						goto setNZCV;

					case MOV_LO_HI:
						reg = r.loReg[Rd3_0] = r.registers[m[Rs3_3]];
						goto fetch;

					case MOV_HI_LO:

						reg = r.registers[m[Rd3_0]] = r.loReg[Rs3_3];

						if (Rd3_0 == HiReg::pc)
							goto branch;

						goto fetch;

					case MOV_HI_HI:
						
						reg = r.registers[m[Rd3_0]] = r.registers[m[Rs3_3]];

						if (Rd3_0 == HiReg::pc)
							goto branch;

						goto fetch;

					//Branch and Exchange
					//1 cycle + 2 cycle prefetch = 3 cycles

					case BX_LO:
						r.pc = r.loReg[Rs3_3];
						goto branchExchange;

					case BX_HI:
						r.pc = r.registers[m[Rs3_3]];
						goto branchExchange;

					default:
						goto undef;

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

	r.cpsr.carry(reg & 0x100000000);
	r.cpsr.overflow((left & i32_MIN) == (right & i32_MIN) && (reg & i32_MIN) != (left & i32_MIN));

setNZ:

	r.cpsr.negative(reg & i32_MIN);
	r.cpsr.zero(reg == 0);

fetch:

	fetchNext<true>(r, memory);

}