#pragma once
#include <sysarch.h>

#include <iostream>
#include <optional>

namespace quip {
	std::string GetLastErrorMessage();
	using uint = unsigned int;

	class Clipboard {
		mutable bool _open{ false };

	public:
		Clipboard() {}

		void clear() const;
		void set(const char*) const;
		void* get() const;

	};

	std::istream& operator>>(std::istream&, Clipboard&);
	std::ostream& operator<<(std::ostream&, const Clipboard&);
}
