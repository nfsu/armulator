#pragma once
#include "types/types.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "utils/math.hpp"

namespace arm {

	enum class AccessFlag : bool {
		READ_ONLY,
		READ_WRITE
	};

	template<typename AddressType>
	struct MemoryRange {

		void *location{};
		String name, altName;

		AddressType start, end, size;
		AccessFlag access;

		MemoryRange(AddressType start, AddressType size, AccessFlag access, String name, String altName):
			start(start), size(size), end(start + size), access(access), name(name), altName(altName) {}

		inline bool intersects(const MemoryRange &other) const {
			return (start <= other.start && end > other.start) || (other.start <= start && other.end > end);
		}

		inline bool contains(AddressType address) const {
			return address >= start && address < end;
		}

		inline void *map(AddressType address) const {
			return (u8*)location + address - start;
		}

	};

	template<typename AddressType, typename = std::enable_if<std::is_unsigned_v<AddressType> && std::is_integral_v<AddressType>>>
	class Memory {

	public:

		using Range = MemoryRange<AddressType>;

		Memory(const List<Range> &ranges): ranges(ranges) {

			static_assert(sizeof(AddressType) <= sizeof(usz), "32-bit architectures can't support 64 architectures");

			for (usz i = 0, j = ranges.size(); i < j; ++i)
				for (usz k = 0; k < j; ++k)
					if (i != k && ranges[k].intersects(ranges[i]))
						oic::System::log()->fatal("Virtual memory ranges intersect");

			usz total{};

			for (const Range &range : ranges)
				total += range.size;

			memory = std::move(Buffer(total));
			total = 0;

			for (Range &range : this->ranges) {
				range.location = memory.data() + total;
				total += range.size;
			}

		}

		inline u8 *addr(AddressType ptr) const {

			for (const Range &r : ranges)
				if (r.contains(ptr))
					return (u8*) r.map(ptr);

			oic::System::log()->fatal("Couldn't map range");
			return nullptr;
		}

		inline const Range &mapRange(AddressType ptr) const {

			for (const Range &r : ranges)
				if (r.contains(ptr))
					return r;

			oic::System::log()->fatal("Couldn't map range");
			return *ranges.end();
		}

		template<typename T = u8, AccessFlag access>
		inline T *map(AddressType ptr) const {

			for (const Range &r : ranges)
				if (r.contains(ptr)) {

					if constexpr (access == AccessFlag::READ_WRITE) {
						if (access > r.access) {
							oic::System::log()->fatal("Memory access violation");
							return nullptr;
						}
					}

					return (T*) r.map(ptr);
				}

			oic::System::log()->fatal("Couldn't map range");
			return nullptr;
		}

		//Copies variable into the address (read)
		template<typename T>
		__forceinline T &get(AddressType ptr, T &t) const {
			return t = *map<T, AccessFlag::READ_ONLY>(ptr);
		}

		//Gets the variable from the address (read)
		template<typename T>
		__forceinline const T &get(AddressType ptr) const {
			return *map<T, AccessFlag::READ_ONLY>(ptr);
		}

		//Sets the variable at the address (write)
		template<typename T>
		__forceinline T &set(AddressType ptr, const T &t) {
			return *map<T, AccessFlag::READ_WRITE>(ptr) = t;
		}

		~Memory() = default;

		Memory(const Memory&) = delete;
		Memory(Memory&&) = delete;
		Memory &operator=(const Memory&) = delete;
		Memory &operator=(Memory&&) = delete;

		const List<Range> &getRanges() const { return ranges; }

		const Range &getRange(const String &name) const {

			for (const Range &r : ranges)
				if (r.name == name)
					return r;

			oic::System::log()->fatal("Can't find range");
			return *ranges.end();
		}

		const Range &operator[](usz i) const {
			return ranges[i];
		}

		//The currently selected range
		const Range *selected{};

	private:

		Buffer memory;
		List<Range> ranges;

	};

	using Memory8 = Memory<u8>;
	using Memory16 = Memory<u16>;
	using Memory32 = Memory<u32>;
	using Memory64 = Memory<u64>;

}