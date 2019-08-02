#include "arm/armulator.hpp"
using namespace arm;

Armulator::Armulator(const List<Memory32::Range> &ranges): memory(ranges) {
	r.cpsr.value = 0xD3;		//Initialize cpsr; no FIQ, no IRQ, SVC mode on ARM
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

template<Armulator::Version v, Armulator::DebugType type>
__INLINE__ void wait(Registers &r, Memory32 &memory) {

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
	
	u64 timings[128]{}, timing, instruction{};
	u64 timer;

	if constexpr ((type & Armulator::USE_CYCLE) == 0) {
		timings;
		timing;
	}

	if constexpr ((type & Armulator::USE_TIMER) == 0) {
		timer;
		instruction;
	}

	u64 cycles{};

	if constexpr((type & Armulator::USE_TIMER) != 0)
		timer = std::chrono::high_resolution_clock::now().time_since_epoch().count();

	while (true) {

		#ifdef __USE_EXIT__
			if (r.pc == 0x4) {

				if constexpr ((type & Armulator::USE_TIMER) != 0){
				
					timer = std::chrono::high_resolution_clock::now().time_since_epoch().count() - timer;

					if constexpr((type & Armulator::USE_CYCLE) != 0)
						for(usz i = 0; i < instruction; ++i)
							printf("%zu %llu\n", i, timings[i]);

					printf("%llu cycles (%llu instructions): %lluns (%fns/c, %fns/i)\n",
						   cycles, instruction, timer, f64(timer) / cycles, f64(timer) / instruction);

				}

				throw std::exception();
			}
		#endif

			if constexpr ((type & Armulator::PRINT_INSTRUCTION) != 0 && (v & Armulator::VersionSpec::T) != 0) {
				if (r.cpsr.thumb())
					printThumb<v>(r);
			}

		if constexpr ((type & Armulator::USE_CYCLE) != 0)
			timing = __rdtsc();

		if constexpr((v & Armulator::VersionSpec::T) != 0) {

			if (r.cpsr.thumb())
				step<true, v>(r, memory, hirMap, cycles);
			else
				step<false, v>(r, memory, hirMap, cycles);

		} else
			step<false, v>(r, memory, hirMap, cycles);

		if constexpr ((type & Armulator::USE_CYCLE) != 0)
			timings[instruction] = __rdtsc() - timing;
			
		if constexpr ((type & Armulator::USE_TIMER) != 0)
			++instruction;
	}
}

//Expand template function with switch table
//Avoiding having to include everything in the headers

#define _Type(x, y) case y: ::wait<x, DebugType(y)>(r, memory); break;
#define _Type2(x, y) _Type(x, y) _Type(x, y + 1)
#define _Version(x) switch(u32(debug)) { _Type2(x, 0) _Type2(x, 2) _Type2(x, 4) _Type2(x, 6) } break;

void Armulator::wait(Armulator::Version v, DebugType debug) {

	switch (v) {

		case Armulator::ARM7TDMI:
			_Version(ARM7TDMI);

		case Armulator::ARM9TDMI:
			_Version(ARM9TDMI);

	}

}