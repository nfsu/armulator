#pragma once
#include "types/types.hpp"
#include "memory.hpp"

namespace arm {

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

		__INLINE__ void push(const MemoryRange<AddressSpace> &range, AddressSpace &sp, T &a) {

			if constexpr (!isEmpty) {
				sp += increment;
				*(T*)range.map(sp) = a;
			} else {
				*(T*)range.map(sp) = a;
				sp += increment;
			}

		}

		template<typename ...args>
		__INLINE__ void push(const MemoryRange<AddressSpace> &range, AddressSpace &sp, T &a, args &...arg) {
			push(range, sp, a);
			push(range, sp, arg...);
		}

		template<typename ...args>
		__INLINE__ void push(Memory<AddressSpace> &memory, AddressSpace &sp, args &...arg) {

			const MemoryRange<AddressSpace> &range = memory.mapRange(sp);

			if (range.access == AccessFlag::READ_ONLY)
				oic::System::log()->fatal("Can't push to a readonly address");

			push(range, sp, arg...);
		}

		__INLINE__ void pop(const MemoryRange<AddressSpace> &range, AddressSpace &sp, T &a) {

			if constexpr (!isEmpty) {
				a = *(T*)range.map(sp);
				sp -= increment;
			} else {
				sp -= increment;
				a = *(T*)range.map(sp);
			}

		}

		template<typename ...args>
		__INLINE__ void pop(const MemoryRange<AddressSpace> &range, AddressSpace &sp, T &a, args &...arg) {
			pop(range, sp, a);
			pop(range, sp, arg...);
		}

		template<typename ...args>
		__INLINE__ void pop(Memory<AddressSpace> &memory, AddressSpace &sp, args &...arg) {
			pop(memory.mapRange(sp), sp, arg...);
		}

	};

	//Different stack types

	template<bool isAscending, bool isEmpty>
	using Stack32 = Stack<u32, u32, isAscending, isEmpty>;

	template<bool isAscending, bool isEmpty>
	using Stack64 = Stack<u64, u64, isAscending, isEmpty>;

}