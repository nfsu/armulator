#include "arm/armulator.hpp"
using namespace arm;

Armulator::Armulator(const List<Memory32::Range> &ranges, DebugLevel level, const Buffer &rom, u32 entry, Mode::E mode):
	rom(rom), debugLevel(level), memory(ranges), stack(&memory) {
	r.cpsr.thumb = entry & 1;
	r.cpsr.mode = mode;
	r.pc = entry;
}

Memory32 &Armulator::getMemory() { return memory; }
Stack32<0, 0> &Armulator::getStack(){ return stack;}
Registers &Armulator::getRegisters() { return r; }

void Armulator::wait() {

	#ifdef __ALLOW_DEBUG__
		printPSR(r.cpsr);
	#endif

	while (step());
}

void Armulator::print() {

	u8 modeId = Mode::toId(r.cpsr.mode);
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

}

void Armulator::printPSR(PSR psr) {
	printf("M: %s, T: %u, F: %u, I: %u, V: %u, C: %u, Z: %u, N: %u\n",
		   Mode::name(psr.mode), psr.thumb, psr.disableFiq, psr.disableIrq, psr.overflow, psr.carry, psr.zero, psr.negative);
}

#ifdef __ALLOW_DEBUG__

	void Armulator::printModified() {
	
		u8 prevId = Mode::toId(p.cpsr.mode);
		const u8 *mapping = Registers::mapping[prevId];
	
		for (usz i = 0; i < lr; ++i) {
	
			u32 v = r.registers[mapping[i]];
			u32 vp = p.registers[mapping[i]];
	
			if (v != vp)
				printf("r%zu = %u (%i)\n", i, v, i32(v));
		}
	
		if (p.registers[mapping[lr]] != r.registers[mapping[lr]])
			printf("lr = %u\n", r.registers[mapping[lr]]);
	
		if (p.registers[mapping[sp]] != r.registers[mapping[sp]])
			printf("sp = %u\n", r.registers[mapping[sp]]);
	
		if (p.pc != r.pc)
			printf("pc = %u\n", r.registers[pc /* pc remains shared */]);
	
		if (p.cpsr.value != r.cpsr.value) {
			printf("cpsr = ");
			printPSR(r.cpsr);
		}
	
		if (p.cpsr.mode != r.cpsr.mode) {
			printf("spsr = ");
			printPSR(r.spsr[Mode::toId(r.cpsr.mode)]);
		}
	}

#endif

bool Armulator::step() {

	#ifdef __ALLOW_DEBUG__

		p = r;

	#endif

	u8 prevId = Mode::toId(r.cpsr.mode);
	const u8 *mapping = Registers::mapping[prevId];

	bool modifyCondition = true;
	u32 val = r.cpsr.thumb ? stepThumb(mapping, modifyCondition) : stepArm(mapping, modifyCondition);

	if (modifyCondition) {	//Modify the condition in cpsr

		r.cpsr.zero = val == 0;
		r.cpsr.negative = val & u32(i32_MIN);

		//r.cpsr.carry = ???;
		//r.cpsr.overflow = ???;

	}

	#ifdef __ALLOW_DEBUG__

		if (debugLevel == DebugLevel::ALL)
			print();
		else if (debugLevel == DebugLevel::MODIFIED)
			printModified();

	#endif

	//Step one instruction

	u32 incr = 4 - 2 * r.cpsr.thumb;
	r.pc += incr;

	//Bounds check
	return r.pc < rom.size() && r.pc - incr < r.pc;
}

u32 Armulator::stepArm(const u8 *, bool &) {
	oic::System::log()->fatal("oopsies");
	return u32(-1);
}