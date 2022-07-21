#pragma once
#include <strmath.hpp>

namespace quip {
	/**
	 * @class	HexSequencer
	 * @brief	Manager object for retrieving unique filenames.
	 */
	class HexSequencer {
		using uint = size_t;

		uint _count{ 0ull };

		uint getNext() { return ++_count; }

	public:
		HexSequencer(uint const& off) : _count{ off } {}

		std::string get()
		{
			return str::fromBase10(std::to_string(getNext()), 16);
		}
	};
}
