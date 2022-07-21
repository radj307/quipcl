#pragma once
#include "File.hpp"
#include "HexSequencer.hpp"

#include <fileio.hpp>
#include <fileutil.hpp>
#include <str.hpp>

#include <deque>
#include <filesystem>

namespace quip {
	/**
	 * @class	History
	 * @brief	Manages clipboard history.
	 */
	class History {
		std::filesystem::path _path;
		std::deque<File> _cache;
		HexSequencer _sequencer;

		/**
		 * @brief			Refreshes the given file cache from the given directory.
		 * @param files		Reference of a deque containing the currently-cached files.
		 * @param path		The location of the target directory.
		 * @returns			The reference of the 'files' parameter.
		 */
		static std::deque<File>& refreshAllFiles(std::deque<File>& files, std::filesystem::path const& path, const bool& includeSymlinks = false)
		{
			for (std::filesystem::directory_iterator it{ path }, end{}; it != end; ++it) {
				if (it->is_regular_file() && (!includeSymlinks || !it->is_symlink()) && std::find_if(files.begin(), files.end(), [&it](auto const& file) { return file.path == it->path(); }) == files.end()) {
					files.emplace_back(File{ it->path() });
				}
			}

			files.shrink_to_fit();
			std::sort(files.begin(), files.end(), [](auto&& l, auto&& r) { return l.last_write_time() > r.last_write_time(); });

			return files;
		}
		/**
		 * @brief		Gets all of the files present in the given path.
		 * @param path	The location of the target directory.
		 * @returns		A deque containing all of the files in path.
		 */
		static std::deque<File> getAllFiles(std::filesystem::path const& path, const bool& includeSymlinks = false)
		{
			std::deque<File> files;

			for (std::filesystem::recursive_directory_iterator it{ path }, end{}; it != end; ++it)
				if (it->is_regular_file() && (!includeSymlinks || !it->is_symlink()))
					files.emplace_back(File{ it->path() });

			files.shrink_to_fit();
			std::sort(files.begin(), files.end(), [](auto&& l, auto&& r) { return l.last_write_time() > r.last_write_time(); });

			return files;
		}

		/**
		 * @brief		Gets all of the files present in the cache directory.
		 * @returns		A deque containing all of the files in the cache directory.
		 */
		std::deque<File> getAllFiles() const
		{
			if (!file::exists(_path))
				std::filesystem::create_directories(_path);
			return getAllFiles(_path);
		}

		/**
		 * @brief		Gets the largest index present in the history directory.
		 * @returns		The base-10 representation of the largest hex index (filename) present in the file cache.
		 */
		size_t getLargestCachedIndex() const
		{
			size_t largest{ 0ull };
			bool fst{ true };
			for (const auto& it : _cache) {
				const size_t& n{ str::toBase10<size_t>(it.name(), 16) };
				if (fst) {
					fst = false;
					largest = n;
				}
				else if (n > largest)
					largest = n;
			}
			return largest;
		}

	public:
		History(std::filesystem::path const& path, bool const& initCache = true) : _path{ path }, _cache{ initCache ? getAllFiles() : std::deque<File>{} }, _sequencer{ getLargestCachedIndex() } {}

		/// @brief	Deletes all cache files, including the directory where they are located.
		int delete_all()
		{
			_cache.clear();
			return std::filesystem::remove_all(_path);
		}

		/// @brief	Delete all cache files with a filetime older than the given threshold.
		int delete_older_than(const std::filesystem::file_time_type& time_threshold)
		{
			refresh();
			int count{ 0 };
			for (size_t i{ 0ull }, end{ _cache.size() }; i < end; ++i) {
				const auto& file{ _cache[i] };
				if (file.last_write_time() > time_threshold) {
					if (file.exists() && !std::filesystem::remove(file.path))
						throw make_exception("Failed to remove file at '", file.path, "'!");
					count = _cache.size() - i;
					// remove until the end of the cache, then break since refresh() sorts by last modified.
					_cache.erase(_cache.begin() + i, _cache.end());
					break;
				}
			}
			return count;
		}

		/// @brief	Gets the location of this file on disk.
		std::filesystem::path path() const { return _path; }

		/// @brief	Refreshes the cache from the filesystem.
		void refresh()
		{
			refreshAllFiles(_cache, _path);
		}

		/// @brief	Push a new entry to the cache.
		template<var::Streamable... Ts>
		bool push(Ts&&... data)
		{
			if (!file::exists(_path))
				std::filesystem::create_directories(_path);
			if (const auto& filepath{ _path / _sequencer.get() }; file::write(filepath, std::forward<Ts>(data)...)) {
				_cache.emplace_front(File{ filepath });
				return true;
			}
			return false;
		}

		/// @brief	Retrieves the latest cache data.
		std::optional<std::stringstream> get_latest() const
		{
			if (!_cache.empty())
				return _cache.front().get();
			return std::nullopt;
		}
		/// @brief	Gets the File associated with the given filename.
		std::optional<File> get(const std::string& name) const
		{
			if (const auto& it{ std::find_if(_cache.begin(), _cache.end(), [&name](auto&& f) { return f.path.filename() == name; }) }; it != _cache.end())
				return *it;
			return std::nullopt;
		}
		/// @brief	Gets the File at the given index.
		std::optional<File> get(const size_t& age_index) const
		{
			if (age_index < _cache.size())
				return _cache.at(age_index);
			return std::nullopt;
		}
		/// @brief	Gets the File at the given index.
		std::optional<File> operator[](const size_t& age_index) const
		{
			return get(age_index);
		}
		/// @brief	Gets the (first) File with the given filetime.
		std::optional<File> get(const std::filesystem::file_time_type& file_time) const
		{
			if (const auto& it{ std::find_if(_cache.begin(), _cache.end(), [&file_time](auto&& f) { return f.last_write_time() == file_time; }) }; it != _cache.end())
				return *it;
			return std::nullopt;
		}
		/// @brief	Gets the (first) File with the given filetime.
		std::optional<File> operator[](const std::filesystem::file_time_type& file_time) const
		{
			return get(file_time);
		}

		auto size() const { return _cache.size(); }
		auto begin() const { return _cache.begin(); }
		auto end() const { return _cache.end(); }
		auto rbegin() const { return _cache.rbegin(); }
		auto rend() const { return _cache.rend(); }
	};
}
