#pragma once
#include "armulator.hpp"
#include "values.hpp"
#include "helper.hpp"

namespace arm {

	/*template<Armulator::Version v, bool immediate>
	_inline_ bool dataProc(Registers &r, emu::Memory32 &mem, const u8 *&m, u64 &c) {

		switch (Op7_21) {

			case AND:
				_Rd4_12 = arm::op<arm::and>(S1_20, r.cpsr, _Rn4_16, Op2);
				return false;

			case EOR:
				_Rd4_12 = arm::op<arm::eor>(S1_20, r.cpsr, _Rn4_16, Op2);
				return false;

			default:
				return true;
		}
	}*/

	template<Armulator::Version v>
	_inline_ void stepArm(Registers &r, emu::Memory32 &, const u8 *&, usz &) {

		//Conditional 
		if (!arm::doCondition(arm::cond::Condition(Cond4_28), r.cpsr))
			return;

	//	switch (Op4_24) {

	//		//Data processing

	//		case DATA_PROC0:
	//			if (dataProc<v, false>(r, mem, m, c)) goto undef;
	//			goto checkPc;

	//		case DATA_PROC1:
	//			if (dataProc<v, true>(r, mem, m, c)) goto undef;
	//			goto checkPc;


	//		//Lower 24 bits for SWI code

	//		case SWI:
	//			arm::exception<false, arm::Exception::SWI>(r, mem, c, m);
	//			return;

	//		default:
	//			goto undef;
	//	}

	//checkPc:

	//	if (Rd4_12 == arm::pc) {
	//		arm::branch<false, false>(r, mem, c, m);
	//		return;
	//	}

	//fetch:
	//	arm::fetchNext<false>(r, mem);
	//	return;

	//undef:
	//	arm::exception<false, arm::Exception::UND>(r, mem, c, m);

	}

}