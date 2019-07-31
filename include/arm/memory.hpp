#pragma once
#include "types/types.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "utils/math.hpp"

#define __INLINE__  __forceinline

namespace arm {

	template<typename AddressType>
	struct MemoryRange {

		String name, altName;
		Buffer initMemory;

		AddressType start, size;
		bool write;

		MemoryRange(AddressType start, AddressType size, bool write, String name, String altName, Buffer initMemory):
			start(start), size(size), write(write), name(name), altName(altName),
			initMemory(initMemory) {}

	};

	template<
		typename AddressType, typename =
		std::enable_if<std::is_unsigned_v<AddressType> && std::is_integral_v<AddressType>>
	>
	class Memory {

	public:

		using Range = MemoryRange<AddressType>;

		static constexpr usz mapping = usz(0x80000000);

	private:

		static void allocate();
		static void allocate(Range &r);
		static void free();
		static void free(Range &r);

	public:

		Memory(const List<Range> &ranges_): ranges(ranges_) {

			static_assert(sizeof(AddressType) <= sizeof(usz), "32-bit architectures can't support 64 architectures");

			allocate();

			for (Range &range : ranges)
				allocate(range);
		}

		~Memory() {

			free();

			for (Range &range : ranges)
				free(range);
		}

		//Gets the variable from the address (read)
		template<typename T>
		__INLINE__ const T &get(AddressType ptr) const {
			return *(T*)(mapping | ptr);
		}

		//Sets the variable at the address (write)
		template<typename T>
		__INLINE__ T &set(AddressType ptr, const T &t) {
			return *(T*)(mapping | ptr) = t;
		}

		Memory(const Memory&) = delete;
		Memory(Memory&&) = delete;
		Memory &operator=(const Memory&) = delete;
		Memory &operator=(Memory&&) = delete;

		const List<Range> &getRanges() const { return ranges; }

	private:

		List<Range> ranges;

	};

	using Memory16 = Memory<u16>;
	using Memory32 = Memory<u32>;
	using Memory64 = Memory<u64>;

	#ifdef _WIN32

		namespace win {
			#include <Windows.h>
		}

		template<typename AddressType, typename T>
		void Memory<AddressType, T>::allocate() {

			if (!win::VirtualAlloc(win::LPVOID(mapping), mapping, MEM_RESERVE, PAGE_READWRITE))
				oic::System::log()->fatal("Couldn't reserve memory");

		}

		template<typename AddressType, typename T>
		void Memory<AddressType, T>::allocate(Range &r) {

			win::LPVOID ou{};
			usz map = mapping | r.start;

			if ((ou = win::VirtualAlloc(win::LPVOID(map), usz(r.size), MEM_COMMIT, PAGE_READWRITE)) == nullptr)
				oic::System::log()->fatal("Couldn't allocate memory");

			memset(ou, 0, r.size);

			if (r.initMemory.size() <= r.size) {
				if (r.initMemory.size() != 0) {
					memcpy(ou, r.initMemory.data(), r.initMemory.size());
					r.initMemory.clear();
				}
			} else
				oic::System::log()->fatal("Couldn't initialize memory");

			if (!r.write) {
				win::DWORD oldProtect;
				win::VirtualProtect(ou, usz(r.size), PAGE_READONLY, &oldProtect);
			}
		}

		template<typename AddressType, typename T>
		void Memory<AddressType, T>::free(Range &r) {
			win::VirtualFree(win::LPVOID(mapping | r.start), 0, MEM_DECOMMIT);
		}

		template<typename AddressType, typename T>
		void Memory<AddressType, T>::free() {
			win::VirtualFree(win::LPVOID(mapping), mapping, MEM_RELEASE);
		}

	#else
		//TODO:
	#endif

}