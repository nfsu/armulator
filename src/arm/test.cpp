#include "arm/thumb/instructions.hpp"
using namespace arm::thumb;

static const TI instructions[] = {

	add(r0, r0, 5),		//a = 5
	add(r1, r1, 9),		//b = 9

	lsl(r2, r0, 2),		//c = 5 << 2 = 20
	lsr(r3, r1, 3),		//d = 9 >> 3 = 1
	asr(r4, r1, 1),		//e = 1 >> 1 = 0

	add(r4, r3, r2),	//e = 20 + 1 = 21
	sub(r4, r4, r0),	//e = 21 - 5 = 16
	sub(r3, r3, 12)		//d = 1 - 12 = -11

	//TODO: Write this for every instruction (except main memory access)

};

int main() {

	for (TI instruction : instructions)
		printf("%u %p\n", instruction, (void*)instruction);

	return 0;
}