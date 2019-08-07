#pragma once
#include "mode.hpp"

namespace arm {
	
	//Complexity of getter & setter operations is given in i,j
	//i = instruction cycles
	//j = fetch/store cycles

	struct PSR {

		//Value

		u32 value;

		//Masks

		static constexpr u32
			mMask = 0x1F, tMask = 0x20, fMask = 0x40, iMask = 0x80,
			vMask = 0x10000000, cMask = 0x20000000, zMask = 0x40000000, nMask = 0x80000000;

		//Getters (1i,1j)

		__forceinline Mode::E mode() const { return Mode::E(value & mMask); }	//Execution mode
		__forceinline bool thumb() const { return value & tMask; }				//T; if thumb mode is used
		__forceinline bool disableFIQ() const { return value & fMask; }			//F: if FIQs aren't allowed
		__forceinline bool disableIRQ() const { return value & iMask; }			//I: if IRQs aren't allowed

		//V: the last operation caused a signed overflow
		__forceinline bool overflow() const { return value & vMask; }

		//C: the last operation caused an unsigned overflow
		__forceinline bool carry() const { return value & cMask; }

		//Z: the last operation was zero
		__forceinline bool zero() const { return value & zMask; }

		//N: the last operation was negative
		__forceinline bool negative() const { return value & nMask; }

		//Bool setters (2-3i, 1j)

		//beq 2					; Skip two instructions (to clear bit)
		//orr rx, mask			; Set bit
		//b 1					; Skip one instruction (to end)
		//and rx, notMask		; Clear bit

		//orrne rx, mask
		//andeq rx, notMask

		__forceinline void thumb(bool b) { b ? value |= tMask : value &= ~tMask; }
		__forceinline void clearThumb() { value &= ~tMask; }
		__forceinline void setThumb() { value |= tMask; }

		__forceinline void irq(bool b) { b ? value |= iMask : value &= ~iMask; }
		__forceinline void clearIrq() { value &= ~iMask; }
		__forceinline void setIrq() { value |= iMask; }

		__forceinline void fiq(bool b) { b ? value |= fMask : value &= ~fMask; }
		__forceinline void clearFiq() { value &= ~fMask; }
		__forceinline void setFiq() { value |= fMask; }

		__forceinline void overflow(bool b) { b ? value |= vMask : value &= ~vMask; }
		__forceinline void carry(bool b) { b ? value |= cMask : value &= ~cMask; }
		__forceinline void zero(bool b) { b ? value |= zMask : value &= ~zMask; }
		__forceinline void negative(bool b) { b ? value |= nMask : value &= ~nMask; }

		__forceinline void setCodes(u32 a) {
			negative(i32(a) < 0);
			zero(a == 0);
		}

		__forceinline void setALU(u32 a, u32 b, u32 c) {
			overflow((a & i32_MIN) == (b & i32_MIN) && (a & i32_MIN) != (c & i32_MIN));
			carry(c < a);
			setCodes(c);
		}

		//Mode setter (2i, 2j)

		//and rx, notMask
		//orr rx, mode

		__forceinline void mode(Mode::E mode) { value &= ~mMask; value |= mode; }

	};

}