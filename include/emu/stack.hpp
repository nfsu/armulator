#pragma once
#include "types/types.hpp"
#include "memory.hpp"

namespace emu {

	//The stack class; taking a type T (can even be floats)
	//AddressSpace is the type for the address space
	//This is an interface for how stacks work in low-level architectures and doesn't provide memory management
	//If ascending = true, the stack pointer will increase if you push (memory address grows), otherwise it will decrease
	//If isEmpty = true, the stack pointer will point to the next address where something should be pushed. Otherwise points to last object pushed
	template<
		typename T, typename AddressSpace,
		bool isAscending = false, bool isEmpty = false
	>
	class Stack {

	public:

		static constexpr AddressSpace increment = !isAscending ? (~AddressSpace(sizeof(T))) + 1 : AddressSpace(sizeof(T));

		_inline_ static void push(Memory<AddressSpace> &m, AddressSpace &sp, const T &a) {

			if constexpr (!isEmpty) {
				sp += increment;
				m.set(sp, a);
			} else {
				m.set(sp, a);
				sp += increment;
			}

		}

		template<typename ...args>
		_inline_ static void push(Memory<AddressSpace> &m, AddressSpace &sp, const T &a, const args &...arg) {
			push(m, sp, a);
			push(m, sp, arg...);
		}

		_inline_ static void pop(const Memory<AddressSpace> &m, AddressSpace &sp, T &a) {

			if constexpr (!isEmpty) {
				a = m.get<T>(sp);
				sp -= increment;
			} else {
				sp -= increment;
				a = m.get<T>(sp);
			}

		}

		template<typename ...args>
		_inline_ static void pop(const Memory<AddressSpace> &m, AddressSpace &sp, T &a, args &...arg) {
			pop(m, sp, a);
			pop(m, sp, arg...);
		}

	};

	//Different stack types

	template<bool isAscending, bool isEmpty>
	using Stack32 = Stack<u32, u32, isAscending, isEmpty>;

	template<bool isAscending, bool isEmpty>
	using Stack64 = Stack<u64, u64, isAscending, isEmpty>;

}