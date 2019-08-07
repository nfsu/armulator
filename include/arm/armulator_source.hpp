#pragma once
#include "arm/armulator.hpp"
#include "arm/thumb/tharmulator_source.hpp"
#include "arm/arm_instructions.hpp"

namespace arm {

	Armulator::Armulator(const List<emu::Memory32::Range> &ranges): memory(ranges) {
		r.cpsr.value = 0xD3;		//Initialize cpsr; no FIQ, no IRQ, SVC mode on ARM
	}

	void Armulator::print(Registers &r) {

		u8 modeId = Mode::toId(r.cpsr.mode());
		auto mapping = Registers::mapping[modeId];

		for (usz i = 0; i < sp; ++i)
			printf("r%zu = %p\n", i, (void*) usz(r.registers[mapping[i]]));

		printf("sp = %p\n", (void*) usz(r.registers[mapping[sp]]));
		printf("lr = %p\n", (void*) usz(r.registers[mapping[lr]]));
		printf("pc = %p\n", (void*) usz(r.registers[mapping[pc]]));

		printf("cpsr = ");
		printPSR(r.cpsr);

		printf("spsr = ");
		printPSR(r.spsr[modeId]);
		printf("\n");

	}

	void Armulator::printPSR(PSR psr) {
		printf("M: %s, T: %u, F: %u, I: %u, V: %u, C: %u, Z: %u, N: %u\n",
			   Mode::name(psr.mode()), psr.thumb(), psr.disableFIQ(), psr.disableIRQ(),
			   psr.overflow(), psr.carry(), psr.zero(), psr.negative()
		);
	}

	template<bool isThumb, Armulator::Version v>
	_inline_ void step(Registers &r, emu::Memory32 &memory, const u8 *&hirMap, u64 &cycles) {

		//Perform code cached in ir/nir registers

		if constexpr (isThumb)
			stepThumb<v>(r, memory, hirMap, cycles);
		else
			stepArm<v>(r, memory, hirMap, cycles);

		++cycles;

	}

	template<Armulator::Version v, Armulator::DebugType type>
	_inline_ void wait(Registers &r, emu::Memory32 &memory) {

		//High register mappings
		u8 mid = Mode::toId(r.cpsr.mode());

		/* Optimization for thumb; only fetch from reg if hi register is mentioned*/
		const u8 *hirMap = Registers::mapping[mid];

		//Populate instructions

		if (r.cpsr.thumb()) {
			fetchNext<true>(r, memory);
			fetchNext<true>(r, memory);
			hirMap += 8;						//Only needs mapping for the high registers
		} else {
			fetchNext<false>(r, memory);
			fetchNext<false>(r, memory);
		}

		//Run instructions

		usz cycles{};

		while (true) {

			if constexpr ((type & Armulator::PRINT_INSTRUCTION) != 0 && (v & Armulator::VersionSpec::T) != 0) {
				if (r.cpsr.thumb())
					printThumb<v>(r);
			}

			if constexpr ((v & Armulator::VersionSpec::T) != 0) {

				if (r.cpsr.thumb())
					step<true, v>(r, memory, hirMap, cycles);
				else
					step<false, v>(r, memory, hirMap, cycles);

			} else
				step<false, v>(r, memory, hirMap, cycles);

			if constexpr ((type & Armulator::PRINT_REGISTERS) != 0)
				Armulator::print(r);
		}
	}

}