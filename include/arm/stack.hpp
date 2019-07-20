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

		Stack(Memory<AddressSpace> *memory): memory(memory) {}

		__forceinline void push(AddressSpace &sp, T &a) {

			if constexpr (!isEmpty) {
				sp += increment;
				memory->set(sp, a);
			} else {
				memory->set(sp, a);
				sp += increment;
			}

		}

		template<typename ...args>
		__forceinline void push(AddressSpace &sp, T &a, args &...arg) {
			push(sp, a);
			push(sp, arg...);
		}

		__forceinline void pop(AddressSpace &sp, T &a) {

			if constexpr (!isEmpty) {
				memory->get(sp, a);
				sp -= increment;
			} else {
				sp -= increment;
				memory->get(sp, a);
			}

		}

		template<typename ...args>
		__forceinline void pop(AddressSpace &sp, T &a, args &...arg) {
			pop(sp, a);
			pop(sp, arg...);
		}

	private:

		Memory<AddressSpace> *memory;

	};

	//Different stack types

	template<bool isAscending, bool isEmpty>
	using Stack32 = Stack<u32, u32, isAscending, isEmpty>;

	template<bool isAscending, bool isEmpty>
	using Stack64 = Stack<u64, u64, isAscending, isEmpty>;

}