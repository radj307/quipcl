#pragma once
#include "History.hpp"

#include <sysarch.h>

#include <iostream>
#include <optional>

namespace quip {
	using uint = unsigned int;

	/**
	 * @brief	Represents the windows clipboard.
	 */
	class Clipboard {
		void set_raw(const char*) const;

	public:
		mutable History history;
		bool useHistory;

		Clipboard(std::filesystem::path const& history_directory, const bool useHistory, const bool initHistoryCache = true) : history{ history_directory, useHistory && initHistoryCache }, useHistory{ useHistory } {}

		template<var::Streamable... Ts>
		void set(Ts&&...) const;
		std::string get(bool const& = false) const;
		void clear() const;

		friend std::istream& operator>>(std::istream&, Clipboard&);
		friend std::ostream& operator<<(std::ostream&, const Clipboard&);
	};

	extern std::istream& operator>>(std::istream&, Clipboard&);
	extern std::ostream& operator<<(std::ostream&, const Clipboard&);
}
