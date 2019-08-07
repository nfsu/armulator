#pragma once
#include "emu/helper.hpp"
#include "registers.hpp"

namespace arm {

	//Incrementing multiple data instruction
	//bool st; whether it stores or loads
	//miaPos<false> = POP, miaPos<true> = STMIA
	template<typename AddressType, bool st, usz regs = 8>
	_inline_ void miaPos(emu::Memory<AddressType> &mem, usz &cycles, AddressType &ptr, Registers &r) {

		for (usz i = 0; i < regs; ++i)
			if (r.ir & (1 << i)) {

				if constexpr (st)
					mem.set(ptr, r.loReg[i]);
				else
					r.loReg[i] = mem.get<AddressType>(ptr);

				ptr += 4;
				++cycles;
			}
	}

	//Decrementing multiple data instruction
	//bool st; whether it stores or loads
	//miaNeg<false> = LDMIA, miaNeg<true> = PUSH
	template<typename AddressType, bool st, usz regs = 8>
	_inline_ void miaNeg(emu::Memory<AddressType> &mem, usz &cycles, AddressType &ptr, Registers &r) {
		for (usz i = 0; i < regs; ++i)
			if (r.ir & (0x80 >> i)) {

				if constexpr (st)
					mem.set(ptr, r.loReg[7 - i]);
				else
					r.loReg[7 - i] = mem.get<AddressType>(ptr);

				ptr -= 4;
				++cycles;
			}
	}
	
	//Fill next instruction pipeline

	template<bool isThumb>
	_inline_ void fetchNext(Registers &r, emu::Memory32 &memory) {

		r.ir = r.nir;

		if constexpr (isThumb) {
			r.nir = memory.get<u16>(r.pc);
			r.pc += 2;
		} else {
			r.nir = memory.get<u32>(r.pc);
			r.pc += 4;
		}
	}

	//Branch which can change the thumb flag or just continue the current mode
	//Prefetches next instructions

	template<bool thumb, bool exchange, bool forceArm = false>
	_inline_ void branch(Registers &r, emu::Memory32 &mem, usz &cycles, const u8 *&mapping) {

		cycles += 2;

		if constexpr (!exchange) {
			fetchNext<thumb>(r, mem);
			fetchNext<thumb>(r, mem);
			mapping;
		} else {

			if constexpr(!forceArm)
				if (r.pc & 1) {

					r.pc &= ~1;

					if constexpr (!thumb) {
						r.cpsr.setThumb();
						mapping += 8;
					}

					fetchNext<true>(r, mem);
					fetchNext<true>(r, mem);
					return;
				}

			if constexpr (thumb) {
				r.cpsr.clearThumb();
				mapping -= 8;
			}

			fetchNext<false>(r, mem);
			fetchNext<false>(r, mem);

		}
	}

	//Trigger exception and prefetch
	template<bool thumb, Exception e>
	_inline_ void exception(Registers &r, emu::Memory32 &mem, usz &cycles, const u8 *&mapping) {
		r.exception<e>();
		branch<thumb, true, true>(r, mem, cycles, mapping);
	}

	//If the condition should be executed
	_inline_ bool doCondition(cond::Condition c, PSR &psr) {

		using namespace cond;

		switch (c) {

			case EQ:
				return psr.zero();

			case NE:
				return !psr.zero();

			case CS:
				return psr.carry();

			case CC:
				return !psr.carry();

			case MI:
				return psr.negative();

			case PL:
				return !psr.negative();

			case VS:
				return psr.overflow();

			case VC:
				return !psr.overflow();

			case HI:
				return psr.carry() && !psr.zero();

			case LS:
				return !psr.carry() || psr.zero();

			case GE:
				return psr.negative() == psr.overflow();

			case LT:
				return psr.negative() != psr.overflow();

			case GT:
				return psr.negative() && !psr.zero();

			case LE:
				return !psr.negative() || psr.zero();

			case AL:
				return true;

			default:
				return false;
		}

	}

}