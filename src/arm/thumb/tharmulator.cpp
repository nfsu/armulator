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

	#define printOp2i(name, Rd, Rs, i)														\
		if(debugLevel != arm::Armulator::DebugLevel::NONE) 									\
			printf("%s\n", (String(#name) + " r" + num(Rd) + ", r" + num(Rs) + ", #" + num(i)).c_str());

	#define printOp1i(name, Rd, i)															\
		if(debugLevel != arm::Armulator::DebugLevel::NONE) 									\
			printf("%s\n", (String(#name) + " r" + num(Rd) + ", #" + num(i)).c_str());

	#define printOp1(name, Rs)																\
		if(debugLevel != arm::Armulator::DebugLevel::NONE) 									\
			printf("%s\n", (String(#name) + " r" + num(Rs)).c_str());

	#define printOp2(name, Rd, Rs)															\
		if(debugLevel != arm::Armulator::DebugLevel::NONE) 									\
			printf("%s\n", (String(#name) + " r" + num(Rd) + ", r" + num(Rs)).c_str());

	#define printOp3(name, Rd, Rs, Rn)														\
		if(debugLevel != arm::Armulator::DebugLevel::NONE) 									\
			printf("%s\n", (String(#name) + " r" + num(Rd) + ", r" + num(Rs) + ", r" + num(Rn)).c_str());
#else
	#define printOpi(name, ...) 
#endif

//

//m is only used for higher registers
u32 arm::Armulator::stepThumb(const u8 *m, bool &setConditionCodes) {

	u8 *ptr = memory.addr(r.registers[pc] & ~1);
	RegOp5b &i5b = *(RegOp5b*)ptr;

	switch (i5b.op) {

		//RegOp5b

		case LSL:		//Logical shift left
			printOp2i(LSL, i5b.Rd, i5b.Rs, i5b.i);
			return r.registers[i5b.Rd] = r.registers[i5b.Rs] << i5b.i;

		case LSR:		//Logical shift right
			printOp2i(LSR, i5b.Rd, i5b.Rs, i5b.i);
			return r.registers[i5b.Rd] = r.registers[i5b.Rs] >> i5b.i;

		case ASR:		//Arithmetic shift right (maintain sign)
			printOp2i(ASR, i5b.Rd, i5b.Rs, i5b.i);
			return r.registers[i5b.Rd] = arm::asr(r.registers[i5b.Rs], i5b.i);

		case STRi:		//Store u32 (with intermediate offset)
			printOp2i(STR, i5b.Rd, i5b.Rs, i5b.i << 2);
			setConditionCodes = false;
			return memory.set(i5b.Rs + (i5b.i << 2), r.registers[i5b.Rd]);

		case LDRi:		//Load u32 (with intermediate offset)
			printOp2i(LDR, i5b.Rd, i5b.Rs, i5b.i << 2);
			setConditionCodes = false;
			return memory.get(i5b.Rs + (i5b.i << 2), r.registers[i5b.Rd]);

		case STRBi:		//Store u8 (with intermediate offset)
			printOp2i(STRB, i5b.Rd, i5b.Rs, i5b.i);
			setConditionCodes = false;
			return memory.set(i5b.Rs + i5b.i, u8(r.registers[i5b.Rd]));

		case LDRBi:		//Store u8 (with intermediate offset)
			printOp2i(LDRB, i5b.Rd, i5b.Rs, i5b.i);
			setConditionCodes = false;
			return r.registers[i5b.Rd] = memory.get<u8>(i5b.Rs + i5b.i);

		case STRHi:		//Store u16 (with intermediate offset)
			printOp2i(STRH, i5b.Rd, i5b.Rs, i5b.i << 1);
			setConditionCodes = false;
			return memory.set(i5b.Rs + (i5b.i << 1), u16(r.registers[i5b.Rd]));

		case LDRHi:		//Store u16 (with intermediate offset)
			printOp2i(LDRH, i5b.Rd, i5b.Rs, i5b.i << 1);
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
						printOp1i(MOV, i8b.Rd, i8b.i);
						return r.registers[i8b.Rd] = i8b.i;

					case CMP:		//Compare value to constant
						printOp1i(CMP, i8b.Rd, i8b.i);
						return r.registers[i8b.Rd] - i8b.i;

					case ADD:		//Add assign with const
						printOp1i(ADD, i8b.Rd, i8b.i);
						return r.registers[i8b.Rd] += i8b.i;

					case SUB:		//Subtract assign with const
						printOp1i(SUB, i8b.Rd, i8b.i);
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
				RegOp3b &i3b = *(RegOp3b*)ptr;

				switch (i3b.op) {

					case ADD_R:
						printOp3(ADD, i3b.Rd, i3b.Rs, i3b.Rni);
						return r.registers[i3b.Rd] = r.registers[i3b.Rs] + r.registers[i3b.Rni];

					case SUB_R:
						printOp3(ADD, i3b.Rd, i3b.Rs, i3b.Rni);
						return r.registers[i3b.Rd] = r.registers[i3b.Rs] - r.registers[i3b.Rni];

					case ADD_3B:
						printOp2i(ADD, i3b.Rd, i3b.Rs, i3b.Rni);
						return r.registers[i3b.Rd] = r.registers[i3b.Rs] + i3b.Rni;

					case SUB_3B:
						printOp2i(ADD, i3b.Rd, i3b.Rs, i3b.Rni);
						return r.registers[i3b.Rd] = r.registers[i3b.Rs] - i3b.Rni;

					//TODO: STR, STRH, STRB, LDSB, LDR, LDRH, LDRB, LDSH
				}

			}

		//RegOp0b
		case ALU_HI_BX:
			{
				RegOp0b &i0b = *(RegOp0b*) ptr;

				switch (i0b.op) {

					case AND:
						printOp2(AND, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] &= r.registers[i0b.Rs];

					case EOR:
						printOp2(EOR, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] ^= r.registers[i0b.Rs];

					case LSL_R:
						printOp2(LSL, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] <<= r.registers[i0b.Rs];

					case LSR_R:
						printOp2(LSR, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] >>= r.registers[i0b.Rs];

					case ASR_R:
						printOp2(ASR, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] = arm::asr(r.registers[i0b.Rd], r.registers[i0b.Rs]);

					case ADC:
						printOp2(ADC, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] -= r.registers[i0b.Rs] + !r.cpsr.carry;

					case SBC:
						printOp2(SBC, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] += r.registers[i0b.Rs] + r.cpsr.carry;

					case ROR:
						printOp2(ROR, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] = arm::ror(r.registers[i0b.Rd], r.registers[i0b.Rs]);

					case TST:
						printOp2(TST, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] & r.registers[i0b.Rs];

					case NEG:
						printOp2(NEG, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] = u32(-i32(r.registers[i0b.Rs]));

					case CMP_R:
						printOp2(CMP, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] - r.registers[i0b.Rs];

					case CMN:
						printOp2(CMN, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] + r.registers[i0b.Rs];

					case ORR:
						printOp2(ORR, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] |= r.registers[i0b.Rs];

					case MUL:
						printOp2(MUL, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] *= r.registers[i0b.Rs];

					case BIC:
						printOp2(BIC, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] &= ~r.registers[i0b.Rs];

					case MVN:
						printOp2(MVN, i0b.Rd, i0b.Rs);
						return r.registers[i0b.Rd] = ~r.registers[i0b.Rs];

					case ADD_LO_HI:
						printOp2(ADD, i0b.Rd, i0b.Rs | 8);
						return r.registers[i0b.Rd] += r.registers[m[i0b.Rs | 8]];

					case ADD_HI_LO:
						printOp2(ADD, i0b.Rd | 8, i0b.Rs);
						return r.registers[m[i0b.Rd | 8]] += r.registers[i0b.Rs];

					case ADD_HI_HI:
						printOp2(ADD, i0b.Rd | 8, i0b.Rs | 8);
						return r.registers[m[i0b.Rd | 8]] += r.registers[m[i0b.Rs | 8]];

					case CMP_LO_HI:
						printOp2(CMP, i0b.Rd, i0b.Rs | 8);
						return r.registers[i0b.Rd] - r.registers[m[i0b.Rs | 8]];

					case CMP_HI_LO:
						printOp2(CMP, i0b.Rd | 8, i0b.Rs);
						return r.registers[m[i0b.Rd | 8]] - r.registers[i0b.Rs];

					case CMP_HI_HI:
						printOp2(CMP, i0b.Rd | 8, i0b.Rs | 8);
						return r.registers[m[i0b.Rd | 8]] - r.registers[m[i0b.Rs | 8]];

					case MOV_LO_HI:
						printOp2(MOV, i0b.Rd, i0b.Rs | 8);
						return r.registers[i0b.Rd] = r.registers[m[i0b.Rs | 8]];

					case MOV_HI_LO:
						printOp2(MOV, i0b.Rd | 8, i0b.Rs);
						return r.registers[m[i0b.Rd | 8]] = r.registers[i0b.Rs];

					case MOV_HI_HI:
						printOp2(MOV, i0b.Rd | 8, i0b.Rs | 8);
						return r.registers[m[i0b.Rd | 8]] = r.registers[m[i0b.Rs | 8]];

					case BX_LO:
						printOp1(BX, i0b.Rs);
						setConditionCodes = false;
						return r.cpsr.thumb = (r.registers[pc] = r.registers[i0b.Rs]) & 1;

					case BX_HI:
						printOp1(BX, i0b.Rs | 8);
						setConditionCodes = false;
						return r.cpsr.thumb = (r.registers[pc] = r.registers[m[i0b.Rs | 8]]) & 1;

				}

			}

	}

	oic::System::log()->fatal("Unsupported operation");
	return u32(-1);
}