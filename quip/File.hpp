#pragma once
#include <fileio.hpp>
#include <fileutil.hpp>

#include <filesystem>
#include <utility>

namespace quip {
	struct File {
		/**
		 * @struct	Preview
		 * @brief	Simple stream-functor that displays a limited rectangular section of a given stringstream reference.
		 */
		struct Preview {
			mutable std::stringstream buffer;
			std::optional<size_t> maxLength, maxLines;
			bool useEllipsis{ true };
			static constexpr char LINE_DELIMITER{ '\n' };

			Preview(std::stringstream&& buffer, std::optional<size_t> const& maxLength = 120ull, std::optional<size_t> const& maxLines = 2ull, bool const& useEllipsis = true) : buffer{ std::move(buffer) }, maxLength{ maxLength }, maxLines{ maxLines }, useEllipsis{ useEllipsis } {}

			friend std::ostream& operator<<(std::ostream& os, Preview const& p)
			{
				if (p.maxLines != 0ull) {
					bool fst{ true };
					std::string linebuf;

					for (size_t ln{ 0ull }, end{ p.maxLines.value_or(static_cast<size_t>(-1)) };
						 ln < end && std::getline(p.buffer, linebuf, Preview::LINE_DELIMITER);
						 ++ln) {
						if (fst) fst = false;
						else os << '\n';

						if (p.maxLength.has_value() && linebuf.size() > p.maxLength.value())
							os << linebuf.substr(0ull, p.maxLength.value());
						else  os << linebuf;
					}
					if (!p.buffer.eof() && p.useEllipsis) {
						os << "\n(...)";
					}
				}
				return os;
			}
		};

		std::filesystem::path path;

		File(std::filesystem::path const& path) : path{ path } {}
		File(std::filesystem::path&& path) : path{ std::move(path) } {}

		auto last_write_time() const
		{
			return std::filesystem::last_write_time(path);
		}

		std::string name() const
		{
			return path.filename().generic_string();
		}

		bool exists() const
		{
			return file::exists(path);
		}
		/// @brief	Deletes the contents of this file.
		void clear()
		{
			if (std::ofstream ofs{ path, std::ios_base::out | std::ios_base::trunc }; ofs.is_open()) {
				ofs.flush();
				ofs.close();
			}
		}
		template<var::Streamable... Ts>
		bool set(Ts&&... data) const
		{
			return file::write(path, std::forward<Ts>(data)...);
		}
		std::stringstream get() const
		{
			return file::read(path);
		}

		Preview getPreview(std::optional<size_t> const& maxLength, std::optional<size_t> const& maxLines, bool const& useEllipsis) const
		{
			return{ std::move(file::read(path)), maxLength, maxLines, useEllipsis };
		}

		operator std::filesystem::path() const { return path; }

		friend std::istream& operator>>(std::istream& is, const File& f)
		{
			f.set(is.rdbuf());
			return is;
		}
		friend std::ostream& operator<<(std::ostream& os, const File& f)
		{
			return os << f.get().rdbuf();
		}
	};
}
