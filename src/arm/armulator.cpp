#include "arm/armulator.hpp"
using namespace arm;

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

__INLINE__ u32 stepArm(Registers &, Memory32 &, const u8 *&, bool &, u64&, u64&) {
	oic::System::log()->fatal("oopsies");
	return u32_MAX;
}

static constexpr u64 conditionFlag = 0x100000000;

template<bool isThumb>
__INLINE__ void step(
	Registers &r, Memory32 &memory, const u8 *&hirMap, u32 &returnCode, bool &condition, u64 &timer, u64 &cycles) {

	//Reset condition (always set unless specified)
	condition = true;

	//Perform code cached in ir/nir registers

	if constexpr(isThumb)
		returnCode = stepThumb(r, memory, hirMap, condition, timer, cycles);
	else 
		returnCode = stepArm(r, memory, hirMap, condition, timer, cycles);

	//Every instruction is at least 1 cycle
	++cycles;

	//Process condition codes

	if (condition) {
		r.cpsr.negative(returnCode & 0x80000000);
		r.cpsr.zero(returnCode == 0);

		fetchNext<isThumb>(r, memory);
	}
	else if(returnCode)
		fetchNext<isThumb>(r, memory);

}

void Armulator::wait() {

	//Stack

	u32 returnCode;
	bool condition;

	memory.selected = &memory.mapRange(r.pc);

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

	u64 cycles{};
	u64 timer = std::chrono::high_resolution_clock::now().time_since_epoch().count();

	while (true)
		if (r.cpsr.thumb())
			step<true>(r, memory, hirMap, returnCode, condition, timer, cycles);
		else
			step<false>(r, memory, hirMap, returnCode, condition, timer, cycles);
}