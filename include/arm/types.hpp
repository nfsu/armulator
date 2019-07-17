#pragma once
#include "types/bitset.hpp"

namespace arm {

	//A 3-bit value
	using Value3 = oic::Bitset<3, u8>;

	//A 4-bit value
	using Value4 = oic::Bitset<4, u8>;

	//A 5-bit value
	using Value5 = oic::Bitset<5, u8>;

	//A 6-bit value
	using Value6 = oic::Bitset<6, u8>;

	//A 7-bit value
	using Value7 = oic::Bitset<7, u8>;

	//A 10-bit value
	using Value10 = oic::Bitset<10, u16>;

	//A 12-bit value
	using Value12 = oic::Bitset<12, u16>;

	//A 23-bit value
	using Value23 = oic::Bitset<23, u32>;

}