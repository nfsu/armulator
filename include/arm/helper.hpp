#pragma once
#include "types/types.hpp"
#include "condition.hpp"

namespace arm {

	//Setting condition flags
	_inline_ void setCondition(PSR &cpsr, u32 c) {
		cpsr.negative(c & i32_MIN);
		cpsr.zero(c == 0);
	}

	//Setting CV (and possibly NZ)
	//greater is set if c should be greater than a; needed for setting carry
	template<bool greater>
	_inline_ void setCV(PSR &cpsr, u32 a, u32 b, u32 c) {

		cpsr.overflow((a & i32_MIN) == (b & i32_MIN) && (a & i32_MIN) != (c & i32_MIN));

		if constexpr(greater)
			cpsr.carry(c < a);
		else
			cpsr.carry(c > a);

		setCondition(cpsr, c);
	}

	//LSL (<<)
	template<bool S = true>
	_inline_ u32 lsl(PSR &cpsr, u32 a, u32 b) {

		if (b)
			cpsr.carry(a & (i32_MIN >> (b - 1)));

		u32 c = a << b;
		if constexpr (S) setCondition(cpsr, c);
		return c;
	}

	template<bool S = true>
	_inline_ void lslInto(PSR &cpsr, u32 &a, u32 b) { a = lsl<S>(cpsr, a, b); }

	template<bool S = true>
	_inline_ void mulInto(PSR &cpsr, u32 &a, u32 b) {
		a *= b; cpsr;
		if constexpr (S) setCondition(cpsr, a);
	}

	//LSR (>>)
	template<bool S = true>
	_inline_ u32 lsr(PSR &cpsr, u32 a, u32 b) {

		if (b)
			cpsr.carry(a & (1 << (b - 1)));

		u32 c = a >> b;
		if constexpr (S) setCondition(cpsr, c);
		return c;
	}

	template<bool S = true>
	_inline_ void lsrInto(PSR &cpsr, u32 &a, u32 b) { a = lsr<S>(cpsr, a, b); }

	//Shift right, maintain sign
	template<bool S = true>
	_inline_ u32 asr(PSR &cpsr, u32 a, u32 b) {

		if (b)
			cpsr.carry(a & (1 << (b - 1)));

		u32 c = a & i32_MIN ? ~(~a >> b) : a >> b;
		if constexpr (S) setCondition(cpsr, c);
		return c;
	}

	template<bool S = true>
	_inline_ void asrInto(PSR &cpsr, u32 &a, u32 b) { a = asr<S>(cpsr, a, b); }

	//Shift right, maintain shifted
	template<bool allowOver32, bool S = true>
	_inline_ u32 ror(PSR &cpsr, u32 a, u32 b) {

		if constexpr (allowOver32)
			b %= 32;

		if (b)
			cpsr.carry(a & (1 << (b - 1)));

		u32 c = (a >> b) | (a << (32 - b));
		if constexpr (S) setCondition(cpsr, c);
		return c;
	}

	template<bool S = true>
	_inline_ void rorInto(PSR &cpsr, u32 &a, u32 b) { a = ror<false, S>(cpsr, a, b); }

	//Move into register
	template<bool S = true>
	_inline_ void mov(PSR &cpsr, u32 &a, u32 b) {
		a = b; cpsr;
		if constexpr (S) setCondition(cpsr, b);
	}

	//Sub two values
	template<bool S = true>
	_inline_ u32 sub(PSR &cpsr, u32 a, u32 b) {
		u32 c = a - b; cpsr;
		if constexpr (S) setCV<false>(cpsr, a, b, c);
		return c;
	}

	template<bool S = true>
	_inline_ void subFrom(PSR &cpsr, u32 &a, u32 b) {
		a = sub<S>(cpsr, a, b);
	}

	//Add two values
	template<bool S = true>
	_inline_ u32 add(PSR &cpsr, u32 a, u32 b) {
		u32 c = a + b; cpsr;
		if constexpr (S) setCV<true>(cpsr, a, b, c);
		return c;
	}

	template<bool S = true>
	_inline_ void addTo(PSR &cpsr, u32 &a, u32 b) {
		a = add<S>(cpsr, a, b);
	}

	//And two values
	template<bool S = true>
	_inline_ u32 and(PSR &cpsr, u32 a, u32 b) {
		u32 c = a & b; cpsr;
		if constexpr (S) setCondition(cpsr, c);
		return c;
	}

	//And two values into a
	template<bool S = true>
	_inline_ void andInto(PSR &cpsr, u32 &a, u32 b) {
		u32 c = a & b; cpsr;
		if constexpr (S) setCondition(cpsr, c);
		a = c;
	}

	//Or two values
	template<bool S = true>
	_inline_ void orrInto(PSR &cpsr, u32 &a, u32 b) {
		u32 c = a | b; cpsr;
		if constexpr (S) setCondition(cpsr, c);
		a = c;
	}

	//Xor two values
	template<bool S = true>
	_inline_ void eorInto(PSR &cpsr, u32 &a, u32 b) {
		u32 c = a ^ b; cpsr;
		if constexpr (S) setCondition(cpsr, c);
		a = c;
	}

	//Incrementing multiple data instruction
	//bool st; whether it stores or loads
	//miaPos<false> = POP, miaPos<true> = STMIA
	template<bool st, usz regs = 8>
	_inline_ void miaPos(Memory32 &mem, u64 &cycles, u32 &ptr, Registers &r) {

		for (usz i = 0; i < regs; ++i)
			if (r.ir & (1 << i)) {

				if constexpr(st)
					mem.set(ptr, r.loReg[i]);
				else
					r.loReg[i] = mem.get<u32>(ptr);

				ptr += 4;
				++cycles;
			}
	}

	//Decrementing multiple data instruction
	//bool st; whether it stores or loads
	//miaNeg<false> = LDMIA, miaNeg<true> = PUSH
	template<bool st, usz regs = 8>
	_inline_ void miaNeg(Memory32 &mem, u64 &cycles, u32 &ptr, Registers &r) {
		for (usz i = 0; i < regs; ++i)
			if (r.ir & (0x80 >> i)) {

				if constexpr (st)
					mem.set(ptr, r.loReg[7 - i]);
				else
					r.loReg[7 - i] = mem.get<u32>(ptr);

				ptr -= 4;
				++cycles;
			}
	}

	_inline_ void strb(Memory32 &mem, u32 val, u32 dst, u32 off) { mem.set(dst + off, u8(val)); }
	_inline_ void strh(Memory32 &mem, u32 val, u32 dst, u32 off) { mem.set(dst + off, u16(val)); }
	_inline_ void str(Memory32 &mem, u32 val, u32 dst, u32 off) { mem.set(dst + off, val); }
	_inline_ void ldrb(Memory32 &mem, u32 &val, u32 dst, u32 off) { val = mem.get<u8>(dst + off); }
	_inline_ void ldrh(Memory32 &mem, u32 &val, u32 dst, u32 off) { val = mem.get<u16>(dst + off); }
	_inline_ void ldr(Memory32 &mem, u32 &val, u32 dst, u32 off) { val = mem.get<u32>(dst + off); }

	//Copy signed byte into register
	_inline_ void ldsb(Memory32 &mem, u32 &val, u32 dst, u32 off) { 
		u32 v = mem.get<u8>(dst + off);
		val = v | (v & 0x80 * (0xFFFFFF00 / 0x80));
	}

	//Copy signed short into register
	_inline_ void ldsh(Memory32 &mem, u32 &val, u32 dst, u32 off) { 
		u32 v = mem.get<u16>(dst + off);
		val = v | (v & 0x8000 * (0xFFFF0000 / 0x8000));
	}

	//Fill next instruction pipeline

	template<bool isThumb>
	_inline_ void fetchNext(Registers &r, Memory32 &memory) {

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
	_inline_ void branch(Registers &r, Memory32 &mem, u64 &cycles, const u8 *&mapping) {

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
	_inline_ void exception(Registers &r, Memory32 &mem, u64 &cycles, const u8 *&mapping) {
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