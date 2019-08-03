#pragma once
#include "types/types.hpp"
#include "../armulator.hpp"
#include "opcodes.hpp"
#include <unordered_map>
#include <sstream>

namespace arm::thumb {

	//Names of instructions

	using Name = const c8[6];

	static constexpr Name names5[] = {
		"LSL",	"LSR",	"ASR",	"",
		"MOV",	"CMP",	"ADD",	"SUB",
		"",		"LDR",	"",		"",
		"STR",	"LDR",	"STRB", "LDRB",
		"STRH", "LDRH", "STR",	"LDR",
		"ADD",	"ADD",	"ADD",	"",
		"LDMIA","STMIA","",		"",
		"B",	"BLX",	"",		"BL"
	};

	static constexpr Name conditions[] = {
		"BEQ", "BNE", "BCS", "BCC",
		"BMI", "BPL", "BVS", "BVC",
		"BHI", "BLS", "BGE", "BLT",
		"BGT", "BLE", "BAL", "SWI"
	};

	static constexpr Name names4[] = {
		"PUSH", "PUSH",	"",		"",
		"",		"",		"",		"",
		"POP",	"POP",	"BKPT",	""
	};

	static ::std::unordered_map<usz, const c8*> names7 {
		{ ADD_3B, "ADD" },
		{ ADD_R, "ADD" },
		{ SUB_3B, "SUB" },
		{ SUB_R, "SUB" },
		{ STR, "STR" },
		{ STRH, "STRH" },
		{ STRB, "STRB" },
		{ LDSB, "LDSB" },
		{ LDR, "LDR" },
		{ LDRH, "LDRH" },
		{ LDRB, "LDRB" },
		{ LDSH, "LDSH" }
	};

	static ::std::unordered_map<usz, const c8*> names10 {
		{ AND,			"AND" },
		{ EOR,			"EOR" },
		{ LSL_R,		"LSL" },
		{ LSR_R,		"LSR" },
		{ ASR_R,		"ASR" },
		{ ADC, 			"ADC" },
		{ SBC, 			"SBC" },
		{ ROR, 			"ROR" },
		{ TST, 			"TST" },
		{ NEG, 			"NEG" },
		{ CMP_R,		"CMP" },
		{ CMN,			"CMN" },
		{ ORR,			"ORR" },
		{ MUL,			"MUL" },
		{ BIC,			"BIC" },
		{ MVN,			"MVN" },
		{ ADD_LO_HI,	"ADD" },
		{ ADD_HI_LO,	"ADD" },
		{ ADD_HI_HI,	"ADD" },
		{ CMP_LO_HI,	"CMP" },
		{ CMP_HI_LO,	"CMP" },
		{ CMP_HI_HI,	"CMP" },
		{ MOV_LO_HI,	"MOV" },
		{ MOV_HI_LO,	"MOV" },
		{ MOV_HI_HI,	"MOV" },
		{ BX_LO,		"BX" },
		{ BX_HI,		"BX" },
	};

	//Print functions

	bool print(std::stringstream &ss) {
		oic::System::log()->debug(ss.str());
		return true;
	}

	bool printUndef(std::stringstream &ss, u32 instr) {
		ss << "DATA #" << instr;
		return print(ss);
	}

	bool print(std::stringstream &ss, const c8 *s, i32 intermediate) {
		ss << s << " #" << intermediate;
		return print(ss);
	}

	bool printr(std::stringstream &ss, const c8 *s, u32 Rd) {
		ss << s << " r" << Rd;
		return print(ss);
	}

	bool print(std::stringstream &ss, const c8 *s, u32 Rd, u32 Rs, i32 intermediate) {
		ss << s << " r" << Rd << ", r" << Rs << ", #" << intermediate;
		return print(ss);
	}

	bool printr(std::stringstream &ss, const c8 *s, u32 Rd, u32 Rs, u32 Rb) {
		ss << s << " r" << Rd << ", r" << Rs << ", r" << Rb;
		return print(ss);
	}

	bool print(std::stringstream &ss, const c8 *s, u32 Rd, i32 intermediate) {
		ss << s << " r" << Rd << ", #" << intermediate;
		return print(ss);
	}

	bool printr(std::stringstream &ss, const c8 *s, u32 Rd, u32 Rs) {
		ss << s << " r" << Rd << ", r" << Rs;
		return print(ss);
	}

	bool printv(std::stringstream &ss, const c8 *op, u32 Rd, u8 v, u32 Rs) {

		bool prev = false;
		ss << op << " r" << Rd << " { ";

		for (usz i = 0; i < 8; ++i)
			if (v & (1 << i)) {

				if (prev) ss << ", ";
				else prev = true;

				ss << 'r' << i;
			}

		ss << (prev ? ", r" : "r") << Rs << " }";
		return print(ss);
	}

	bool printv(std::stringstream &ss, const c8 *op, u32 Rd, u8 v) {

		bool prev = false;
		ss << op << " r" << Rd << " { ";

		for (usz i = 0; i < 8; ++i)
			if (v & (1 << i)) {

				if (prev) ss << ", ";
				else prev = true;

				ss << 'r' << i;
			}

		ss << " }";
		return print(ss);
	}

	//Print instruction set

	template<arm::Armulator::Version v>
	_inline_ bool printThumb(arm::Registers &r) {

		std::stringstream ss;

		switch (Op5_11 /* fetch first 5 bits of opcode */) {

			case LSL:
			case LSR:
			case ASR:
			case STRBi:
			case LDRBi:
				return print(ss, names5[Op5_11], Rd3_0, Rs3_3, i5_6);

			case STRi:
			case LDRi:
				return print(ss, names5[Op5_11], Rd3_0, Rs3_3, i5_6_2);

			case STRHi:
			case LDRHi:
				return print(ss, names5[Op5_11], Rd3_0, Rs3_3, i5_6_1);

			case MOV:
			case CMP:
			case ADD:
			case SUB:
				return print(ss, names5[Op5_11], Rd3_8, i8_0);

			case STR_SP:
			case LDR_SP:
			case ADD_SP:
				return print(ss, names5[Op5_11], Rd3_8, arm::sp, i8_0_2);

			case LDR_PC:
			case ADD_PC:
				return print(ss, names5[Op5_11], Rd3_8, arm::pc, i8_0_2);

			case INCR_SP:
				return print(ss, names5[Op5_11], arm::sp, r.ir & 0x80 ? -i32(i7_0_2) : i7_0_2);

			case STMIA:
			case LDMIA:
				return printv(ss, names5[Op5_11], Rd3_8, i8_0);

			case B:
				return print(ss, names5[Op5_11], i32(s12));

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
						return print(ss, conditions[Op8_8 & 0xF], u32(i8(i8_0)) << 1);

					case SWI:
						return print(ss, conditions[Op8_8 & 0xF], i8_0);

					case PUSH:
					case POP:
						return printv(ss, names4[Op8_8 - PUSH], arm::sp, i8_0);

					case PUSH_LR:
						return printv(ss, names4[Op8_8 - PUSH], arm::sp, i8_0, lr);

					case POP_PC:
						return printv(ss, names4[Op8_8 - PUSH], arm::sp, i8_0, pc);

					case BKPT:

						if constexpr ((v & 0xFF) >= arm::Armulator::VersionSpec::v5)
							return print(ss, names4[Op8_8 - PUSH], i8_0);

					default:
						goto undef;
				}

			case BLX:

				if constexpr ((v & 0xFF) < arm::Armulator::VersionSpec::v5)
					goto undef;

			case BLL:
				return print(ss, names5[Op5_11], s23);

			case ADD_SUB:
			case ST:
			case LD:

				switch (Op7_9) {

					case ADD_R:
					case SUB_R:
					case STR:
					case STRH:
					case STRB:
					case LDSB:
					case LDR:
					case LDRH:
					case LDRB:
					case LDSH:
						return printr(ss, names7[Op7_9], Rd3_0, Rs3_3, Rni3_6);

					case ADD_3B:
					case SUB_3B:
						return print(ss, names7[Op7_9], Rd3_0, Rs3_3, i32(Rni3_6));
						
					default:
						goto undef;
				}

			case ALU_HI_BX:

				switch (Op10_6) {

					case AND:
					case EOR:
					case LSL_R:
					case LSR_R:
					case ASR_R:
					case ADC:
					case SBC:
					case ROR:
					case TST:
					case NEG:
					case CMP_R:
					case CMN:
					case ORR:
					case MUL:
					case BIC:
					case MVN:
						return printr(ss, names10[Op10_6], Rd3_0, Rs3_3);

					case ADD_LO_HI:
					case CMP_LO_HI:
					case MOV_LO_HI:
						return printr(ss, names10[Op10_6], Rd3_0 | 8, Rs3_3);

					case ADD_HI_LO:
					case CMP_HI_LO:
					case MOV_HI_LO:
						return printr(ss, names10[Op10_6], Rd3_0, Rs3_3 | 8);

					case ADD_HI_HI:
					case CMP_HI_HI:
					case MOV_HI_HI:
						return printr(ss, names10[Op10_6], Rd3_0 | 8, Rs3_3 | 8);

					case BX_LO:
						return printr(ss, names10[Op10_6], Rd3_0);

					case BX_HI:
						return printr(ss, names10[Op10_6], Rd3_0 | 8);

					default:
						goto undef;
				}
		}

	undef:
		return printUndef(ss, r.ir);
	}

}