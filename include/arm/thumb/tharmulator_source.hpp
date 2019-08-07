#pragma once
#include "arm/armulator.hpp"
#include "arm/thumb/instructions.hpp"
#include "arm/helper.hpp"
#include "arm/thumb/values.hpp"
#include "arm/thumb/debug.hpp"
#include "emu/stack.hpp"

//Step through a thumb instruction
//Where m is the mapping of high registers (type mapping + 8) so m[HiReg] matches the register id it should fetch from
//Normal instructions take 1 cycle

namespace arm::thumb {

	template<
		arm::Armulator::Version v, bool ascendingStack = false, bool emptyStack = false
	>
	_inline_ void stepThumb(arm::Registers &r, emu::Memory32 &memory, const u8 *&m, usz &cycles) {

		using Stack = emu::Stack32<ascendingStack, emptyStack>;

		switch (Op5_11 /* fetch first 5 bits of opcode */) {

			//Move shifted register (LSL/LSR/ASR)
			//1S cycle
			//Layout: Rd3_0, Rs3_3, i5_6, Op5_11

			case LSL:		//Logical shift left
				r.loReg[Rd3_0] = emu::lsl(r.cpsr, r.loReg[Rs3_3], i5_6);
				break;

			case LSR:		//Logical shift right
				r.loReg[Rd3_0] = emu::lsr(r.cpsr, r.loReg[Rs3_3], i5_6);
				break;

			case ASR:		//Arithmetic shift right (maintain sign)
				r.loReg[Rd3_0] = emu::asr(r.cpsr, r.loReg[Rs3_3], i5_6);
				break;

				//STR: 2N
				//LDR: 1S + 1N + 1I

			case STRi:		//Store u32 (with intermediate offset)
				++cycles;
				emu::str(memory, r.loReg[Rd3_0], r.loReg[Rs3_3], i5_6_2);
				break;

			case LDRi:		//Load u32 (with intermediate offset)
				cycles += 2;
				emu::ldr(memory, r.loReg[Rd3_0], r.loReg[Rs3_3], i5_6_2);
				break;

			case STRBi:		//Store u8 (with intermediate offset)
				++cycles;
				emu::strb(memory, r.loReg[Rd3_0], r.loReg[Rs3_3], i5_6);
				break;

			case LDRBi:		//Store u8 (with intermediate offset)
				cycles += 2;
				emu::ldrb(memory, r.loReg[Rd3_0], r.loReg[Rs3_3], i5_6);
				break;

			case STRHi:		//Store u16 (with intermediate offset)
				++cycles;
				emu::strh(memory, r.loReg[Rd3_0], r.loReg[Rs3_3], i5_6_1);
				break;

			case LDRHi:		//Store u16 (with intermediate offset)
				cycles += 2;
				emu::ldrh(memory, r.loReg[Rd3_0], r.loReg[Rs3_3], i5_6_1);
				break;

				//Layout:
				//i8_0, Rd3_8, Op5_11

			case MOV:
				emu::mov(r.cpsr, r.loReg[Rd3_8], i8_0);
				break;

			case CMP:
				emu::sub(r.cpsr, r.loReg[Rd3_8], i8_0);
				break;

			case ADD:
				emu::addTo(r.cpsr, r.loReg[Rd3_8], i8_0);
				break;

			case SUB:
				emu::subFrom(r.cpsr, r.loReg[Rd3_8], i8_0);
				break;

			case STR_SP:
				emu::str(memory, r.loReg[Rd3_8], r.registers[m[HiReg::sp]], i8_0_2);
				break;

			case LDR_SP:
				emu::ldr(memory, r.loReg[Rd3_8], r.registers[m[HiReg::sp]], i8_0_2);
				break;

			case LDR_PC:
				emu::ldr(memory, r.loReg[Rd3_8], r.pc & ~3, i8_0_2);
				break;

			case ADD_PC:
				r.loReg[Rd3_8] = r.pc + i8_0_2;
				break;

			case ADD_SP:
				r.loReg[Rd3_8] = r.registers[m[HiReg::sp]] + i8_0_2;
				break;

			case INCR_SP:
				r.registers[m[HiReg::sp]] += r.ir & 0x80 ? u32(-i32(i7_0_2)) : i7_0_2;
				break;

				//Load/store multiple
				//LDMIA takes 2 + n cycles
				//STMIA takes 1 + n cycles

			case STMIA:
				arm::miaPos<u32, true>(memory, cycles, r.loReg[Rd3_8], r);
				break;

			case LDMIA:
				++cycles;
				arm::miaNeg<u32, false>(memory, cycles, r.loReg[Rd3_8], r);
				break;

				//Unconditional (thumb) branch
				//Interpret 11-bit 2's complement as u32
				//Assembler takes into account prefetch
				//Takes 3 cycles

			case B:
				r.pc += s12;
				arm::branch<true, false>(r, memory, cycles, m);
				return;

				//Conditional (thumb) branch
				//Assembler takes into account prefetch
				//Takes 3 cycles

			case B0:
			case B1:
			case PUSH_POP:

				switch (Op8_8) {

					case BEQ:
					case BNE:
					case BCS:
					case BCC:
					case BMI:
					case BPL:
					case BVS:
					case BVC:
					case BHI:
					case BLS:
					case BGE:
					case BLT:
					case BGT:
					case BLE:
					case BAL:

						if (arm::doCondition(arm::cond::Condition(Op8_8 & 0xF), r.cpsr)) {
							r.pc += u32(i8(i8_0)) << 1;
							arm::branch<true, false>(r, memory, cycles, m);
							return;
						}

						break;

					case SWI:
						arm::exception<true, arm::Exception::SWI>(r, memory, cycles, m);
						return;

						//Push and pop instructions
						//Assuming r0 is located closest to the top and lr is located furthest from the top
					case PUSH_LR:

						Stack::push(memory, r.registers[m[HiReg::sp]], r.registers[m[HiReg::lr]] | 1);
						++cycles;

					case PUSH:

						arm::miaNeg<u32, true>(memory, cycles, r.loReg[Rd3_8], r);
						break;

					case POP_PC:
						++cycles;

					case POP:

						arm::miaPos<u32, false>(memory, cycles, r.loReg[Rd3_8], r);

						if (r.ir & 0x100) {
							Stack::pop(memory, r.registers[m[HiReg::sp]], r.pc);
							arm::branch<true, (v & 0xFF) >= arm::Armulator::VersionSpec::v5>(r, memory, cycles, m);
							return;
						}

						break;

					case BKPT:

						if constexpr ((v & 0xFF) >= arm::Armulator::VersionSpec::v5) {
							arm::exception<true, arm::Exception::PREFETCH_ABORT>(r, memory, cycles, m);
							return;
						}

					default:
						goto undef;
				}

				break;

				//Long (thumb) branch with link
				//Fetch nir and construct 23-bit two's complement and jump to it
				//Assembler takes into account prefetch
				//Takes 4 cycles
			case BLL:
				++cycles;
				r.registers[m[HiReg::lr]] = (r.pc - 2) | 1;		//Next instruction into LR
				r.pc += s23;
				arm::branch<true, false>(r, memory, cycles, m);
				return;

				//Long branch with link
				//Fetch nir and construct 23-bit two's complement and jump to it
				//Assembler takes into account prefetch
				//Takes 4 cycles
			case BLX:

				if constexpr ((v & 0xFF) >= arm::Armulator::VersionSpec::v5) {

					++cycles;

					r.registers[m[HiReg::lr]] = (r.pc - 2) | 1;		//Next instruction into LR
					r.pc += s23;

					arm::branch<true, true, true>(r, memory, cycles, m);
					return;

				} else
					goto undef;

				//Layout:
				//Rd3_0, Rs3_3, Rni3_6, Op7_9

			case ADD_SUB:
			case ST:
			case LD:

				switch (Op7_9) {

					case ADD_R:
						r.loReg[Rd3_0] = emu::add(r.cpsr, r.loReg[Rs3_3], r.loReg[Rni3_6]);
						break;

					case SUB_R:
						r.loReg[Rd3_0] = emu::sub(r.cpsr, r.loReg[Rs3_3], r.loReg[Rni3_6]);
						break;

					case ADD_3B:
						r.loReg[Rd3_0] = emu::add(r.cpsr, r.loReg[Rs3_3], Rni3_6);
						break;

					case SUB_3B:
						r.loReg[Rd3_0] = emu::sub(r.cpsr, r.loReg[Rs3_3], Rni3_6);
						break;

					case STR:
						emu::str(memory, r.loReg[Rd3_0], r.loReg[Rs3_3], r.loReg[Rni3_6]);
						break;

					case STRH:
						emu::strh(memory, r.loReg[Rd3_0], r.loReg[Rs3_3], r.loReg[Rni3_6]);
						break;

					case STRB:
						emu::strb(memory, r.loReg[Rd3_0], r.loReg[Rs3_3], r.loReg[Rni3_6]);
						break;

					case LDSB:
						emu::ldsb(memory, r.loReg[Rd3_0], r.loReg[Rs3_3], r.loReg[Rni3_6]);
						break;

					case LDR:
						emu::ldr(memory, r.loReg[Rd3_0], r.loReg[Rs3_3], r.loReg[Rni3_6]);
						break;

					case LDRH:
						emu::ldrh(memory, r.loReg[Rd3_0], r.loReg[Rs3_3], r.loReg[Rni3_6]);
						break;

					case LDRB:
						emu::ldrb(memory, r.loReg[Rd3_0], r.loReg[Rs3_3], r.loReg[Rni3_6]);
						break;

					case LDSH:
						emu::ldsh(memory, r.loReg[Rd3_0], r.loReg[Rs3_3], r.loReg[Rni3_6]);
						break;

					default:
						goto undef;
				}

				break;

				//Layout:
				//Rd3_0, Rs3_3, Op10_6

			case ALU_HI_BX:

				switch (Op10_6) {

					case AND:
						emu::andInto(r.cpsr, r.loReg[Rd3_0], r.loReg[Rs3_3]);
						break;

					case EOR:
						emu::eorInto(r.cpsr, r.loReg[Rd3_0], r.loReg[Rs3_3]);
						break;

					case LSL_R:
						++cycles;
						emu::lslInto(r.cpsr, r.loReg[Rd3_0], r.loReg[Rs3_3]);
						break;

					case LSR_R:
						++cycles;
						emu::lsrInto(r.cpsr, r.loReg[Rd3_0], r.loReg[Rs3_3]);
						break;

					case ASR_R:
						++cycles;
						emu::asrInto(r.cpsr, r.loReg[Rd3_0], r.loReg[Rs3_3]);
						break;

					case ADC:
						emu::addTo(r.cpsr, r.loReg[Rd3_0], r.loReg[Rs3_3] + r.cpsr.carry());
						break;

					case SBC:
						emu::subFrom(r.cpsr, r.loReg[Rd3_0], r.loReg[Rs3_3] + r.cpsr.carry());
						break;

					case ROR:
						emu::rorInto(r.cpsr, r.loReg[Rd3_0], r.loReg[Rs3_3]);
						++cycles;
						break;

					case TST:
						emu::and(r.cpsr, r.loReg[Rd3_0], r.loReg[Rs3_3]);
						break;

					case NEG:
						r.loReg[Rd3_0] = emu::sub(r.cpsr, 0_u32, r.loReg[Rs3_3]);
						break;

					case CMP_R:
						emu::sub(r.cpsr, r.loReg[Rd3_0], r.loReg[Rs3_3]);
						break;

					case CMN:
						emu::add(r.cpsr, r.loReg[Rd3_0], r.loReg[Rs3_3]);
						break;

					case ORR:
						emu::orrInto(r.cpsr, r.loReg[Rd3_0], r.loReg[Rs3_3]);
						break;

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

						emu::mulInto(r.cpsr, r.loReg[Rd3_0], r.loReg[Rs3_3]);
						break;

					case BIC:
						emu::andInto(r.cpsr, r.loReg[Rd3_0], ~r.loReg[Rs3_3]);
						break;

					case MVN:
						emu::mov(r.cpsr, r.loReg[Rd3_0], ~r.loReg[Rs3_3]);
						break;

					case ADD_LO_HI:
						emu::addTo<PSR, u32, false>(r.cpsr, r.loReg[Rd3_0], r.registers[m[Rs3_3]]);
						break;

					case ADD_HI_LO:
						emu::addTo<PSR, u32, false>(r.cpsr, r.registers[m[Rd3_0]], r.loReg[Rs3_3]);
						goto checkPc;

					case ADD_HI_HI:
						emu::addTo<PSR, u32, false>(r.cpsr, r.registers[m[Rd3_0]], r.registers[m[Rs3_3]]);
						goto checkPc;

					case CMP_LO_HI:
						emu::sub(r.cpsr, r.loReg[Rd3_0], r.registers[m[Rs3_3]]);
						break;

					case CMP_HI_LO:
						emu::sub(r.cpsr, r.registers[m[Rd3_0]], r.loReg[Rs3_3]);
						break;

					case CMP_HI_HI:
						emu::sub(r.cpsr, r.registers[m[Rd3_0]], r.registers[m[Rs3_3]]);
						break;

					case MOV_LO_HI:
						emu::mov<PSR, u32, false>(r.cpsr, r.loReg[Rd3_0], r.registers[m[Rs3_3]]);
						break;

					case MOV_HI_LO:
						emu::mov<PSR, u32, false>(r.cpsr, r.registers[m[Rd3_0]], r.loReg[Rs3_3]);
						goto checkPc;

					case MOV_HI_HI:
						emu::mov<PSR, u32, false>(r.cpsr, r.registers[m[Rd3_0]], r.registers[m[Rs3_3]]);
						goto checkPc;

					checkPc:

						if (Rd3_0 == HiReg::pc) {
							arm::branch<true, (v & 0xFF) >= arm::Armulator::VersionSpec::v5>(r, memory, cycles, m);
							return;
						}

						break;

						//Branch and Exchange
						//1 cycle + 2 cycle prefetch = 3 cycles

					case BX_LO:
						r.pc = r.loReg[Rs3_3];
						arm::branch<true, true>(r, memory, cycles, m);
						return;

					case BX_HI:
						r.pc = r.registers[m[Rs3_3]];
						arm::branch<true, true>(r, memory, cycles, m);
						return;

					default:
						goto undef;

				}

				break;

			default:
				goto undef;

				//TODO: Aborts; prefetch and data abort
				//TODO: USR mode should enable bios protection, otherwise disable it

		}

		arm::fetchNext<true>(r, memory);
		return;

	undef:
		arm::exception<true, arm::Exception::UND>(r, memory, cycles, m);

	}

}