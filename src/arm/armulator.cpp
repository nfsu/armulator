#include "arm/armulator.hpp"
using namespace arm;

#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

Armulator::Armulator(const List<Memory32::Range> &ranges, u32 entry, Mode::E mode):
	memory(ranges), stack(&memory) {

	r.cpsr.thumb(entry & 1);
	r.cpsr.mode(mode);
	r.pc = entry & ~1;
}

void Armulator::print() {

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

__INLINE__ u32 stepArm(Registers &, Memory32 &, const u8 *&, bool &) {
	oic::System::log()->fatal("oopsies");
	return u32_MAX;
}

//Fill next instruction pipeline

__INLINE__ void fetchNext(Registers &r, Memory32 &memory) {

	r.ir = r.nir;

	if (r.cpsr.thumb()) {
		r.nir = *(u16*) memory.selected->map(r.pc);
		r.pc += 2;
	} else {
		r.nir = *(u32*) memory.selected->map(r.pc);
		r.pc += 4;
	}

}

static constexpr u64 conditionFlag = 0x100000000;

__INLINE__ void step(
	Registers &r, Memory32 &memory, const u8 *&hirMap, u32 &returnCode, bool &condition, u64 &timer,
	u64 *&instructionTime) {

	//For detecting time per instruction

	#ifdef __USE_TIMER__
		timer = __rdtsc();
	#else
		timer;
	#endif

	//Reset condition (always set unless specified)
	condition = true;

	//Perform code cached in ir/nir registers

	if (r.cpsr.thumb())
		returnCode = stepThumb(r, memory, hirMap, condition);
	else 
		returnCode = stepArm(r, memory, hirMap, condition);

	//Process condition codes

	if (condition) {
		r.cpsr.negative(returnCode & 0x80000000);
		r.cpsr.zero(returnCode);
	}

	//Populate instructions

	fetchNext(r, memory);

	//For printing timing

	#ifdef __USE_TIMER__
		*instructionTime = __rdtsc() - timer;
		++instructionTime;
	#endif

	#ifdef __ALLOW_DEBUG__
		//printModified();	TODO:
	#endif

}

void Armulator::wait() {

	//Stack

	u64 timer;
	u32 returnCode;
	bool condition;

	u64 instructionTimes[1024];						//Instructions to be timed
	u64 *instructionTime = instructionTimes;		//Instruction number

	memory.selected = &memory.mapRange(r.pc);

	//Populate instructions

	fetchNext(r, memory);
	fetchNext(r, memory);

	//High register mappings

	u8 mid = Mode::toId(r.cpsr.mode());
	const u8 *hirMap =
		Registers::mapping[mid] +
		r.cpsr.thumb() * 8; /* Optimization for thumb; only fetch from reg if hi register is mentioned*/

	//Run instructions
	try {

		while (true)
			step(r, memory, hirMap, returnCode, condition, timer, instructionTime);

	} catch (std::exception e) {
		for (u64 *inst = instructionTimes; inst < instructionTime; ++inst)
			printf("%llu\n", *inst);
	}
}