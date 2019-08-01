#include "arm/armulator.hpp"
using namespace arm;

#if _WIN32
#include <intrin.h>
#endif

Armulator::Armulator(const List<Memory32::Range> &ranges, u32 entry, Mode::E mode):
	memory(ranges) {

	r.cpsr.thumb(entry & 1);
	r.cpsr.mode(mode);
	r.pc = entry & ~1;
}

void Armulator::print(Registers &r) {

	u8 modeId = Mode::toId(r.cpsr.mode());
	auto mapping = Registers::mapping[modeId];

	for (usz i = 0; i < lr; ++i)
		printf("r%zu = %p\n", i, (void*) usz(r.registers[mapping[i]]));

	printf("lr = %p\n", (void*) usz(r.registers[mapping[lr]]));
	printf("sp = %p\n", (void*) usz(r.registers[mapping[sp]]));
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

#include "../src/arm/thumb/tharmulator.cpp"

template<Armulator::Version>
__INLINE__ void stepArm(Registers &, Memory32 &, const u8 *&, u64&) {
	oic::System::log()->fatal("oopsies");
}

static constexpr u64 conditionFlag = 0x100000000;

template<bool isThumb, Armulator::Version v>
__INLINE__ void step(Registers &r, Memory32 &memory, const u8 *&hirMap, u64 &cycles) {

	//Perform code cached in ir/nir registers

	if constexpr(isThumb)
		stepThumb<v>(r, memory, hirMap, cycles);
	else 
		stepArm<v>(r, memory, hirMap, cycles);

	++cycles;

}

template<Armulator::Version v>
__INLINE__ void wait(Registers &r, Memory32 &memory) {

	//High register mappings

	u8 mid = Mode::toId(r.cpsr.mode());
	const u8 *hirMap =
		Registers::mapping[mid]; /* Optimization for thumb; only fetch from reg if hi register is mentioned*/


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

	u64 cycles{}, instruction{};
	u64 timer;
	
	#ifdef __USE_CYCLE_TIMER__
		u64 timings[128]{}, instruction{}, timing;
	#endif

	#ifdef __USE_TIMER__
		timer = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	#endif

	while (true) {

		#ifdef __USE_EXIT__
			if (r.ir == 0xC0DE) {

				#ifdef __USE_TIMER__
				
					timer = std::chrono::high_resolution_clock::now().time_since_epoch().count() - timer;

					#ifdef __USE_CYCLE_TIMER__
						for(usz i = 0; i < instruction; ++i)
							printf("%zu %llu\n", i, timings[i]);
					#endif

					printf("%llu cycles (%llu instructions): %lluns (%fns/c, %fns/i)\n", cycles, instruction, timer, f64(timer) / cycles, f64(timer) / instruction);

				#endif

				throw std::exception();
			}
		#endif
		
		#ifdef __USE_CYCLE_TIMER__
			timing = __rdtsc();
		#endif

		if constexpr((v & Armulator::VersionSpec::T) != 0) {

			if (r.cpsr.thumb())
				step<true, v>(r, memory, hirMap, cycles);
			else
				step<false, v>(r, memory, hirMap, cycles);

		} else
			step<false, v>(r, memory, hirMap, cycles);

		#ifdef __USE_CYCLE_TIMER__
			timings[instruction] = __rdtsc() - timing;
		#endif

		++instruction;
	}
}

void Armulator::wait(Armulator::Version v) {

	if (v == Armulator::ARM7TDMI)
		::wait<Armulator::ARM7TDMI>(r, memory);
	else
		::wait<Armulator::ARM9TDMI>(r, memory);

}