#pragma once
#include "types/types.hpp"

namespace arm::cond {

	enum Condition : u8 /* 4-bit */ {

		EQ,				//Zero is set (==)
		NE,				//!EQ (!=)
		CS,				//Carry is set (unsigned >=)
		CC,				//!CS (unsigned <)
		MI,				//Negative is set (< 0)
		PL,				//!MI (>= 0)
		VS,				//oVerflow (set)
		VC,				//!VS (clear)
		HI,				//Carry & !Zero (unsigned >)
		LS,				//!HI (unsigned <=)
		GE,				//Negative == Overflow (>=)
		LT,				//!GE (<)
		GT,				//!Zero && Negative == Overflow  (>)
		LE,				//!GT (<=)
		AL				//Always

	};

}