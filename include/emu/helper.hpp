#pragma once
#include "memory.hpp"

namespace emu {

	template<typename T> struct Signed {};
	template<> struct Signed<u8> { using v = i8; };
	template<> struct Signed<u16> { using v = i16; };
	template<> struct Signed<u32> { using v = i32; };
	template<> struct Signed<u64> { using v = i64; };

	template<typename T>
	using Signed_v = typename Signed<T>::v;

	template<typename T>
	constexpr T bitSize = T(sizeof(T) * 8);

	template<typename T>
	constexpr T sign = T(1 << (bitSize<T> - 1));

	//LSL (<<)
	template<typename CPSR, typename T, bool S = true>
	_inline_ T lsl(CPSR &cpsr, T a, T b) {

		if constexpr (S)
			if (b)
				cpsr.carry(a & (sign<T> >> (b - 1)));

		T c = a << b;
		if constexpr (S) cpsr.setCodes(c);
		return c;
	}

	template<typename CPSR, typename T, bool S = true>
	_inline_ void lslInto(CPSR &cpsr, T &a, T b) { a = lsl<CPSR, T, S>(cpsr, a, b); }

	template<typename CPSR, typename T, bool S = true>
	_inline_ void mulInto(CPSR &cpsr, T &a, T b) {
		a *= b; cpsr;
		if constexpr (S) cpsr.setCodes(a);
	}

	//LSR (>>)
	template<typename CPSR, typename T, bool S = true>
	_inline_ T lsr(CPSR &cpsr, T a, T b) {

		if constexpr (S)
			if (b)
				cpsr.carry(a & (1 << (b - 1)));

		T c = a >> b;
		if constexpr (S) cpsr.setCodes(c);
		return c;
	}

	template<typename CPSR, typename T, bool S = true>
	_inline_ void lsrInto(CPSR &cpsr, T &a, T b) { a = lsr<CPSR, T, S>(cpsr, a, b); }

	//Shift right, maintain sign
	template<typename CPSR, typename T, bool S = true>
	_inline_ T asr(CPSR &cpsr, T a, T b) {

		if constexpr (S)
			if (b)
				cpsr.carry(a & (1 << (b - 1)));

		T c = a & sign<T> ? ~(~a >> b) : a >> b;
		if constexpr (S) cpsr.setCodes(c);
		return c;
	}

	template<typename CPSR, typename T, bool S = true>
	_inline_ void asrInto(CPSR &cpsr, T &a, T b) { a = asr<CPSR, T, S>(cpsr, a, b); }

	//Shift right, maintain shifted
	template<typename CPSR, typename T, bool S = true>
	_inline_ T ror(CPSR &cpsr, T a, T b) {

		if constexpr (S)
			if (b)
				cpsr.carry(a & (1 << (b - 1)));

		T c = (a >> b) | (a << (bitSize<T> - b));
		if constexpr (S) cpsr.setCodes(c);
		return c;
	}

	template<typename CPSR, typename T, bool S = true>
	_inline_ void rorInto(CPSR &cpsr, T &a, T b) { a = ror<CPSR, T, S>(cpsr, a, b); }

	//Move into register
	template<typename CPSR, typename T, bool S = true>
	_inline_ void mov(CPSR &cpsr, T &a, T b) {
		a = b; cpsr;
		if constexpr (S) cpsr.setCodes(b);
	}

	//Sub two values
	template<typename CPSR, typename T, bool S = true>
	_inline_ T sub(CPSR &cpsr, T a, T b) {
		T c = a - b; cpsr;
		if constexpr (S) cpsr.setALU(a, T(-Signed_v<T>(b)), c);
		return c;
	}

	template<typename CPSR, typename T, bool S = true>
	_inline_ void subFrom(CPSR &cpsr, T &a, T b) {
		a = sub<CPSR, T, S>(cpsr, a, b);
	}

	//Add two values
	template<typename CPSR, typename T, bool S = true>
	_inline_ T add(CPSR &cpsr, T a, T b) {
		T c = a + b; cpsr;
		if constexpr (S) cpsr.setALU(a, b, c);
		return c;
	}

	template<typename CPSR, typename T, bool S = true>
	_inline_ void addTo(CPSR &cpsr, T &a, T b) {
		a = add<CPSR, T, S>(cpsr, a, b);
	}

	//And two values
	template<typename CPSR, typename T, bool S = true>
	_inline_ T and(CPSR &cpsr, T a, T b) {
		T c = a & b; cpsr;
		if constexpr (S) cpsr.setCodes(c);
		return c;
	}

	//And two values into a
	template<typename CPSR, typename T, bool S = true>
	_inline_ void andInto(CPSR &cpsr, T &a, T b) {
		T c = a & b; cpsr;
		if constexpr (S) cpsr.setCodes(c);
		a = c;
	}

	//Or two values
	template<typename CPSR, typename T, bool S = true>
	_inline_ void orrInto(CPSR &cpsr, T &a, T b) {
		T c = a | b; cpsr;
		if constexpr (S) cpsr.setCodes(c);
		a = c;
	}

	//Xor two values
	template<typename CPSR, typename T, bool S = true>
	_inline_ void eorInto(CPSR &cpsr, T &a, T b) {
		T c = a ^ b; cpsr;
		if constexpr (S) cpsr.setCodes(c);
		a = c;
	}

	template<typename AddressType>
	_inline_ void strb(Memory<AddressType> &mem, AddressType val, AddressType dst, AddressType off) { mem.set(dst + off, u8(val)); }

	template<typename AddressType>
	_inline_ void strh(Memory<AddressType> &mem, AddressType val, AddressType dst, AddressType off) { mem.set(dst + off, u16(val)); }

	template<typename AddressType>
	_inline_ void str(Memory<AddressType> &mem, AddressType val, AddressType dst, AddressType off) { mem.set(dst + off, u32(val)); }

	template<typename AddressType>
	_inline_ void ldrb(Memory<AddressType> &mem, AddressType &val, AddressType dst, AddressType off) { val = mem.get<u8>(dst + off); }

	template<typename AddressType>
	_inline_ void ldrh(Memory<AddressType> &mem, AddressType &val, AddressType dst, AddressType off) { val = mem.get<u16>(dst + off); }

	template<typename AddressType>
	_inline_ void ldr(Memory<AddressType> &mem, AddressType &val, AddressType dst, AddressType off) { val = mem.get<u32>(dst + off); }

	//Copy signed byte into register
	template<typename AddressType>
	_inline_ void ldsb(Memory<AddressType> &mem, AddressType &val, AddressType dst, AddressType off) {
		val = AddressType(Signed_v<AddressType>(mem.get<i8>(dst + off)));
	}

	//Copy signed short into register
	template<typename AddressType>
	_inline_ void ldsh(Memory<AddressType> &mem, AddressType &val, AddressType dst, AddressType off) {
		val = AddressType(Signed_v<AddressType>(mem.get<i16>(dst + off)));
	}

	template<template<bool, typename> typename F, typename CPSR, typename ...args>
	_inline_ auto op(bool b, CPSR &cpsr, args &...arg) {
		if (b)
			return F<true, CPSR>(cpsr, arg...);
		else
			return F<false, CPSR>(cpsr, arg...):
	}

}