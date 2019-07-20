#include "arm/armulator.hpp"
#include "arm/thumb/instructions.hpp"
#include "arm/helper.hpp"
using namespace arm::thumb;

//m is only used for higher registers
u32 arm::Armulator::stepThumb(const u8 *m, bool &setConditionCodes) {

	u8 *ptr = rom.data() + (r.registers[pc] & ~1);
	RegOp5b &i5b = *(RegOp5b*)ptr;

	switch (i5b.op) {

		//RegOp5b

		case LSL:		//Logical shift left
			return r.registers[i5b.Rd] = r.registers[i5b.Rs] << i5b.i;

		case LSR:		//Logical shift right
			return r.registers[i5b.Rd] = r.registers[i5b.Rs] >> i5b.i;

		case ASR:		//Arithmetic shift right (maintain sign)
			return r.registers[i5b.Rd] = arm::asr(r.registers[i5b.Rs], i5b.i);

		case STRi:		//Store u32 (with intermediate offset)
			setConditionCodes = false;
			return memory.set(i5b.Rs + (i5b.i << 2), r.registers[i5b.Rd]);

		case LDRi:		//Load u32 (with intermediate offset)
			setConditionCodes = false;
			return memory.get(i5b.Rs + (i5b.i << 2), r.registers[i5b.Rd]);

		case STRBi:		//Store u8 (with intermediate offset)
			setConditionCodes = false;
			return memory.set(i5b.Rs + i5b.i, u8(r.registers[i5b.Rd]));

		case LDRBi:		//Store u8 (with intermediate offset)
			setConditionCodes = false;
			return r.registers[i5b.Rd] = memory.get<u8>(i5b.Rs + i5b.i);

		case STRHi:		//Store u16 (with intermediate offset)
			setConditionCodes = false;
			return memory.set(i5b.Rs + (i5b.i << 1), u16(r.registers[i5b.Rd]));

		case LDRHi:		//Store u16 (with intermediate offset)
			setConditionCodes = false;
			return r.registers[i5b.Rd] = memory.get<u16>(i5b.Rs + (i5b.i << 1));

		//RegOp8b

		case MOV:
		case CMP:
		case ADD:
		case SUB:

			{
				RegOp8b &i8b = *(RegOp8b*)ptr;

				switch (i5b.op /* identical to i8b.op */) {

					case MOV:		//Move value into register
						return r.registers[i8b.Rd] = i8b.i;

					case CMP:		//Compare value to constant
						return r.registers[i8b.Rd] - i8b.i;

					case ADD:		//Add assign with const
						return r.registers[i8b.Rd] += i8b.i;

					case SUB:		//Subtract assign with const
						return r.registers[i8b.Rd] -= i8b.i;

					//TODO: LDR_PC, STMIA, LDMIA
				}
			}

		//RegOp11b

		case B:
		case BLL:
		case BLH:

			setConditionCodes = false;

			switch (i5b.op) {

				//Unconditional branch
				//Interpret 11-bit 2's complement as u32
				//Take into account prefetch
				case B:
					return r.registers[pc] += (((i5b.v << 1) & 0xFFF) | (0x3FFFFC * (i5b.v & 0x400))) + 2;

				//TODO: BLH/BLL
				//Requires the next instruction :(
			}

		//RegOp3b

		case ADD_SUB:
			{
				RegOp3b &i8b = *(RegOp3b*)ptr;

				switch (i8b.op) {

					case ADD_R:
						return r.registers[i8b.Rd] = r.registers[i8b.Rs] + r.registers[i8b.Rni];

					case SUB_R:
						return r.registers[i8b.Rd] = r.registers[i8b.Rs] - r.registers[i8b.Rni];

					case ADD_3B:
						return r.registers[i8b.Rd] = r.registers[i8b.Rs] + i8b.Rni;

					case SUB_3B:
						return r.registers[i8b.Rd] = r.registers[i8b.Rs] - i8b.Rni;

					//TODO: STR, STRH, STRB, LDSB, LDR, LDRH, LDRB, LDSH
				}

			}

		//RegOp0b
		case ALU_HI_BX:
			{
				RegOp0b &i0b = *(RegOp0b*) ptr;

				switch (i0b.op) {

					case AND:
						return r.registers[i0b.Rd] &= r.registers[i0b.Rs];

					case EOR:
						return r.registers[i0b.Rd] ^= r.registers[i0b.Rs];

					case LSL_R:
						return r.registers[i0b.Rd] <<= r.registers[i0b.Rs];

					case LSR_R:
						return r.registers[i0b.Rd] >>= r.registers[i0b.Rs];

					case ASR_R:
						return r.registers[i0b.Rd] = arm::asr(r.registers[i0b.Rd], r.registers[i0b.Rs]);

					case ADC:
						return r.registers[i0b.Rd] -= r.registers[i0b.Rs] + !r.cpsr.carry;

					case SBC:
						return r.registers[i0b.Rd] += r.registers[i0b.Rs] + r.cpsr.carry;

					case ROR:
						return r.registers[i0b.Rd] = arm::ror(r.registers[i0b.Rd], r.registers[i0b.Rs]);

					case TST:
						return r.registers[i0b.Rd] & r.registers[i0b.Rs];

					case NEG:
						return r.registers[i0b.Rd] = u32(-i32(r.registers[i0b.Rs]));

					case CMP_R:
						return r.registers[i0b.Rd] - r.registers[i0b.Rs];

					case CMN:
						return r.registers[i0b.Rd] + r.registers[i0b.Rs];

					case ORR:
						return r.registers[i0b.Rd] |= r.registers[i0b.Rs];

					case MUL:
						return r.registers[i0b.Rd] *= r.registers[i0b.Rs];

					case BIC:
						return r.registers[i0b.Rd] &= ~r.registers[i0b.Rs];

					case MVN:
						return r.registers[i0b.Rd] = ~r.registers[i0b.Rs];

					case ADD_LO_HI:
						return r.registers[i0b.Rd] += r.registers[m[i0b.Rs | 8]];

					case ADD_HI_LO:
						return r.registers[m[i0b.Rd | 8]] += r.registers[i0b.Rs];

					case ADD_HI_HI:
						return r.registers[m[i0b.Rd | 8]] += r.registers[m[i0b.Rs | 8]];

					case CMP_LO_HI:
						return r.registers[i0b.Rd] - r.registers[m[i0b.Rs | 8]];

					case CMP_HI_LO:
						return r.registers[m[i0b.Rd | 8]] - r.registers[i0b.Rs];

					case CMP_HI_HI:
						return r.registers[m[i0b.Rd | 8]] - r.registers[m[i0b.Rs | 8]];

					case MOV_LO_HI:
						return r.registers[i0b.Rd] = r.registers[m[i0b.Rs | 8]];

					case MOV_HI_LO:
						return r.registers[m[i0b.Rd | 8]] = r.registers[i0b.Rs];

					case MOV_HI_HI:
						return r.registers[m[i0b.Rd | 8]] = r.registers[m[i0b.Rs | 8]];

					case BX_LO:
						setConditionCodes = false;
						return r.cpsr.thumb = (r.registers[pc] = r.registers[i0b.Rs]) & 1;

					case BX_HI:
						setConditionCodes = false;
						return r.cpsr.thumb = (r.registers[pc] = r.registers[m[i0b.Rs | 8]]) & 1;

				}

			}

	}

	oic::System::log()->fatal("Unsupported operation");
	return u32(-1);
}