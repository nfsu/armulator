#pragma once
#include "types/types.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "utils/math.hpp"

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/mman.h>
#endif

//#define __INLINE__
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

		static void initMemory(Range &r, void *ou) {

			memset(ou, 0, r.size);

			if (r.initMemory.size() <= r.size) {
				if (r.initMemory.size() != 0) {
					memcpy(ou, r.initMemory.data(), r.initMemory.size());
					r.initMemory.clear();
				}
			} else
				oic::System::log()->fatal("Couldn't initialize memory");
		}

	public:

		Memory(const List<Range> &ranges_): ranges(ranges_) {

			static_assert(sizeof(AddressType) <= sizeof(usz), "32-bit architectures can't support 64 architectures");

			allocate();

			for (Range &range : ranges)
				if(range.size)
					allocate(range);
		}

		~Memory() {

			free();

			for (Range &range : ranges)
				if(range.size)
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

		template<typename AddressType, typename T>
		void Memory<AddressType, T>::allocate() {

			if (!VirtualAlloc(LPVOID(mapping), mapping, MEM_RESERVE, PAGE_READWRITE))
				oic::System::log()->fatal("Couldn't reserve memory");
		}

		template<typename AddressType, typename T>
		void Memory<AddressType, T>::allocate(Range &r) {

			usz map = mapping | r.start;

			if (!VirtualAlloc(LPVOID(map), usz(r.size), MEM_COMMIT, PAGE_READWRITE))
				oic::System::log()->fatal("Couldn't allocate memory");

			initMemory(r, (void*)map);

			DWORD oldProtect;

			if (!r.write && !VirtualProtect((void*)map, usz(r.size), PAGE_READONLY, &oldProtect))
				oic::System::log()->fatal("Couldn't protect memory");
		}

		template<typename AddressType, typename T>
		void Memory<AddressType, T>::free(Range &r) {
			VirtualFree(LPVOID(mapping | r.start), 0, MEM_DECOMMIT);
		}

		template<typename AddressType, typename T>
		void Memory<AddressType, T>::free() {
			VirtualFree(LPVOID(mapping), mapping, MEM_RELEASE);
		}

	#else

		template<typename AddressType, typename T>
		void Memory<AddressType, T>::allocate() {

			if(!mmap((void*)mapping, mapping, PROT_NONE, MAP_PRIVATE | MAP_FIXED, 0))
				oic::System::log()->fatal("Couldn't reserve memory");
		}

		template<typename AddressType, typename T>
		void Memory<AddressType, T>::allocate(Range &r) {

			usz map = mapping | r.start;
			
			if(!mmap((void*)map, usz(r.size), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, 0))
				oic::System::log()->fatal("Couldn't allocate memory");

			initMemory(r, (void*)map);

			if (!r.write && mprotect((void*)map, usz(r.size), PROT_READ))
				oic::System::log()->fatal("Couldn't protect memory");
		}

		template<typename AddressType, typename T>
		void Memory<AddressType, T>::allocate(Range &r) {
			munmap((void*)(mapping | r.start), mapping);
		}

		template<typename AddressType, typename T>
		void Memory<AddressType, T>::allocate() {
			munmap((void*)mapping, mapping);
		}

	#endif

}