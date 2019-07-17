#include "arm/thumb/instructions.hpp"
using namespace arm::thumb;

template<usz n, typename T>
void print(oic::Bitset<n, T> &v) {

	u64 test{};

	for (usz i = 0; i < v.bufferSize; ++i)
		test |= u64(v.at(i)) << (8 * (v.bufferEnd - i));

	printf("%p\n", (void*)test);
}

int main() {

	oic::Bitset<24, u8> test0 { 0b1010'1111, 0b0010'0011, 0b1101'1011 };
	oic::Bitset<24, u8> test1 { 0b0101'0000, 0b0000'0000, 0b0010'0101 };
	oic::Bitset<4, u8> test2 { true, false, true, false };

	print(test0);
	test0 >>= 4;
	print(test0);
	test0 <<= 4;
	print(test0);
	test0 >>= 8;
	print(test0);
	test0 <<= 8;
	print(test0);
	test0 >>= 12;
	print(test0);
	test0 <<= 12;
	print(test0);
	test0 >>= 14;	//invalid
	print(test0);
	test0 <<= 14;	//invalid
	print(test0);

	print(test2);

	print(test1);
	print(test0);
	test1 &= test0;
	print(test1);


	static const TI instructions[] = {

		add(r0, r0, 5),		//a = 5
		add(r1, 9),			//b = 9

		lsl(r2, r0, 2),		//c = 5 << 2 = 20
		lsr(r3, r1, 3),		//d = 9 >> 3 = 1
		asr(r4, r1, 1),		//e = 1 >> 1 = 0

		add(r4, r3, r2),	//e = 20 + 1 = 21
		sub(r4, r4, r0),	//e = 21 - 5 = 16
		sub(r4, r4, 2),		//e = 16 - 2 = 14
		sub(r3, 12),		//d = 1 - 12 = -11

		mov(r0, 237),		//a = 237
		cmp(r0, 236),		//; sets flags: 237 and 236; >=, >, !=
		ldrPc(r5, 4),		//r5 = 0xDEADBEEF
		b(NE, 16),			//; Jumps to beyond the dead beefs if ne

		0xDEAD, 0xBEEF, 0xDEAD, 0xBEEF,
		0xDEAD, 0xBEEF, 0xDEAD, 0xBEEF,

		and(r0, r5),		//a = 237 & DEADBEEF = ED
		eor(r0, r4),		//a = ED ^ E = E3
		mov(r1, 3),			//b = 3
		lsl(r0, r1),		//a = E3 << 3 = 718
		lsr(r0, r1)			//a = 718 >> 3 = E3

		//TODO: Write this for every instruction (except main memory access)

	};

	for (TI instruction : instructions)
		printf("%u %p\n", instruction, (void*)instruction);

	return 0;
}