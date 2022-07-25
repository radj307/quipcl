#include "rc/version.h"
#include "Clipboard.h"
#include "Config.hpp"

#include <ParamsAPI2.hpp>
#include <TermAPI.hpp>
#include <envpath.hpp>
#include <hasPendingDataSTDIN.h>

#include <iostream>

inline static constexpr int DEFAULT_LIST_COUNT{ 10 };

/**
 * @brief			Stream insertion operator for std::optional.
 * @param os		std::ostream&
 * @param optional	An instance of the std::optional wrapper containing a streamable type.
 * @returns			std::ostream&
 */
template<var::Streamable T> std::ostream& operator<<(std::ostream& os, const std::optional<T>& optional)
{
	if (optional.has_value())
		os << optional.value();
	return os;
}

struct Help {
	const std::string& programName;
	const std::string& topic;
	WINCONSTEXPR Help(const std::string& programName, const std::string& topic) : programName{ programName }, topic{ topic } {}
	friend std::ostream& operator<<(std::ostream& os, const Help& h)
	{
	#ifdef OS_WIN
	#define _QUIP_DESC_EXTRA "  Integrates with the Windows system clipboard.\n\n"
	#else
	#define _QUIP_DESC_EXTRA ""
	#endif
	#define QUIP_HELP_HEADER "QuipCL v" << quip_VERSION_EXTENDED << '\n' << "  Commandline clipboard utility & history manager." << '\n' << '\n' << _QUIP_DESC_EXTRA
		os;
		if (h.topic.empty())
			os
			<< QUIP_HELP_HEADER
			<< "USAGE:\n"
			<< "  " << h.programName << " [OPTIONS]" << '\n'
			<< '\n'
			<< "  This program is intended for use with shell pipe operators, but also accepts input via any number of set options." << '\n'
			<< '\n'
			<< "OPTIONS:\n"
			<< "  -h, --help               Shows this help display, or detailed help for a specific option ." << '\n'
			<< "  -v, --version            Prints the current version number, then exits." << '\n'
			<< "  -q, --quiet              Prevents non-essential console output." << '\n'
			<< "  -O                       Forces a print out of the current clipboard contents, regardless of other options." << '\n'
			<< "  -s, --set <DATA>         Sets clipboard data to the given string argument.  This is an alternative to shell pipes." << '\n'
			<< "  -p, --preview <IDX>      Shows a preview of the specified cache entry.  (0 is current, 1 is previous, etc.)" << '\n'
			<< "  -l, --list [COUNT]       Shows a preview of a number of the most recent clipboard entries.  The default is 10." << '\n'
			<< "  -d, --dim <<WID>:<LEN>>  Changes the dimensions of the history preview area.  Omit a number to remove that limit." << '\n'
			<< "  -r, --recall <IDX>       Recalls the specified cache entry to the clipboard, replacing the current value." << '\n'
			<< "  -c, --cache              Copy the current clipboard contents to the cache." << '\n'
			<< "      --clear-cache        Deletes the entire clipboard history cache." << '\n'
			<< "  -S, --cache-size         Gets the current size of the history cache." << '\n'
			<< "      --write-ini          Creates or overwrites the configuration file with the default values, then exit." << '\n'
			;
		else {
			std::string topic{ str::trim(h.topic) };
			if (str::equalsAny(topic, "s", "set"))
				os
				<< QUIP_HELP_HEADER
				<< "USAGE:\n"
				<< "  " << h.programName << " -s|--set <STRING>" << '\n'
				<< '\n'
				<< "  The set option allows you to specify clipboard data with or without shell pipe operators." << '\n'
				<< "  You can use any number of set options, as well as shell pipe operators, in the same command." << '\n'
				<< "  Input received from STDIN always preceeds input from set commands." << '\n'
				<< '\n'
				<< "EXAMPLES:\n"
				<< "  To set the current clipboard contents to \"Hello World!\", you could use any of these (non-exhaustive) methods:" << '\n'
				<< "    echo \"Hello \" | " << h.programName << " -s=World!" << '\n'
				<< "    echo \"Hello World!\" | " << h.programName << '\n'
				<< "    " << h.programName << " -s='Hello World!'" << '\n'
				;
			else if (str::equalsAny(topic, "p", "preview"))
				os
				<< QUIP_HELP_HEADER
				<< "USAGE:\n"
				<< "  " << h.programName << " -p|--preview <INDEX>" << '\n'
				<< '\n'
				<< "  Shows a preview of cached clipboard data at the specified index." << '\n'
				<< "  Indexes start at 0 (current), and increment by one for each previous entry, ending at the number of extant entries." << '\n'
				<< "  Using this in conjunction with the '-d'/'--dim' option allows you to configure how much of the cached data to show." << '\n'
				<< '\n'
				<< "  You can view a list of previews of the most recent cache entries by using the -l|--list option." << '\n'
				;
			else if (str::equalsAny(topic, "l", "list"))
				os
				<< QUIP_HELP_HEADER
				<< "USAGE:\n"
				<< "  " << h.programName << " -l|--list [COUNT]" << '\n'
				<< '\n'
				<< "  Shows a preview of recent cache entries, starting from the current one.  The default count is " << DEFAULT_LIST_COUNT << ".\n"
				<< "  When the -q|--quiet option is not specified, index numbers are shown before each cache entry." << '\n'
				<< "  Using this in conjunction with the '-d'/'--dim' option allows you to configure how much of the cached data to show." << '\n'
				;
			else if (str::equalsAny(topic, "d", "dim"))
				os
				<< QUIP_HELP_HEADER
				<< "USAGE:\n"
				<< "  " << h.programName << " -d|--dim [<WIDTH>:<LINE_COUNT>]" << '\n'
				<< '\n'
				<< "  Sets the dimensions of the cache preview." << '\n'
				<< "  The default width is " << Config.preview_width << ", while the default line count is " << Config.preview_lines << ".\n"
				<< '\n'
				<< "  When no argument is provided, the width and line count limits are disabled; previews will include the entire entry." << '\n'
				<< "  When an argument is provided & it does NOT include a colon character ':', only the preview width is changed." << '\n'
				<< '\n'
				<< "EXAMPLES:\n"
				<< "  Show the first 80 characters of the first 3 lines of the previous cache entry:" << '\n'
				<< "    " << h.programName << " -d=80:3 -p=1" << '\n'
				<< '\n'
				<< "  To show the 5 most recent cache entries without truncating them:" << '\n'
				<< "    " << h.programName << " -dl=5" << '\n'
				;
			else if (str::equalsAny(topic, "r", "recall"))
				os
				<< QUIP_HELP_HEADER
				<< "USAGE:\n"
				<< "  " << h.programName << " -r|--recall <INDEX>" << '\n'
				<< '\n'
				<< "  Recalls the data at the specified cache index to the clipboard." << '\n'
				<< "  Note that this overwrites the current clipboard data without adding it to the cache." << '\n'
				<< "  You can combine this with the -c|--cache option to cache the current clipboard data before overwriting it." << '\n'
				;
			else throw make_exception("There are no help topics for '", h.topic, "'!");
		}
		return os;
	}
};

#include <ini/MINI.hpp>

int main(const int argc, char** argv)
{
	try {
		std::ios_base::sync_with_stdio(false); //< disable cin <=> STDIO synchronization (disables buffering for cin)

		using namespace opt_literals;
		opt::ParamsAPI2 args{ argc, argv, 's'_req, "set"_req, 'p'_req, "preview"_req, 'l'_opt, "list"_opt, 'd'_req, "dim"_req, 'r'_req, "recall"_req };
		const auto& [programPath, programName] { env::PATH().resolve_split(argv[0]) };

		const auto& configPath{ programPath / (std::filesystem::path{ programName }.replace_extension().generic_string() + ".ini") };

		file::ini::MINI config{
			{ "cache", {
				{ "bEnableHistory", "true" },
			{ "bAutoCache", "false" },
		} },
		};

		if (args.checkopt("write-ini") || args.checkopt("ini-write")) {
			std::filesystem::create_directories(configPath.root_directory());
			if (config.write(configPath))
				std::cout << "Successfully created '" << configPath.generic_string() << "'\n";
			else throw make_exception("Failed to write to config file '", configPath.generic_string(), "'!");
			return 0;
		}

		if (file::exists(configPath))
			config.read(configPath, true);

		const bool enableHistory{ config.checkv_any("cache", "bEnableHistory", [](std::string const& value) { return str::tolower(str::trim(value)) == "true"; }) };
		const bool autoCache{ config.checkv_any("cache", "bAutoCache", [](std::string const& value) { return str::tolower(str::trim(value)) == "true"; }) };

		Config.quiet = args.check_any<opt::Flag, opt::Option>('q', "quiet");

		if (args.check_any<opt::Flag, opt::Option>('h', "help")) {
			std::cout << Help(programName.generic_string(), args.typegetv_any<opt::Flag, opt::Option>().value_or("")) << std::endl;
			return 0;
		}
		else if (args.check_any<opt::Flag, opt::Option>('v', "version")) {
			if (!Config.quiet)
				std::cout << "QuipCL  v";
			std::cout << quip_VERSION_EXTENDED << std::endl;
			return 0;
		}

		// begin

		quip::Clipboard clipboard(programPath / "history", enableHistory);

		bool do_io_step{ true }; //< whether or not to perform the I/O step. (although it only affects output, input is always handled when given)

		// HANDLE CONFIG ARGS:

		if (const auto& dimArg{ args.typegetv_any<opt::Flag, opt::Option>('d', "dim") }; dimArg.has_value()) { // set dimensions from argument:
			const auto& [width, lines] { str::split(dimArg.value(), ':') };

			if (!width.empty()) {
				if (std::all_of(width.begin(), width.end(), str::stdpred::isdigit))
					Config.preview_width = str::stoull(width);
				else throw make_exception("Invalid Preview Dimensions:  '", width, "' isn't a valid number for width!");
			}
			else Config.preview_width = std::nullopt;
			if (!lines.empty()) {
				if (std::all_of(lines.begin(), lines.end(), str::stdpred::isdigit))
					Config.preview_lines = str::stoull(lines);
				else throw make_exception("Invalid Preview Dimensions:  '", lines, "' isn't a valid number for line count!");
			}
			else Config.preview_lines = std::nullopt;
		}
		else if (args.check_any<opt::Flag, opt::Option>('d', "dim")) { // dimensions argument was specified but did not specify an argument:
			Config.preview_width = std::nullopt;
			Config.preview_lines = std::nullopt;
		}


		// HANDLE 'BLOCKING' ARGS:

		// Show list of previews
		if (args.check_any<opt::Flag, opt::Option>('l', "list")) {
			do_io_step = false;

			int count{ DEFAULT_LIST_COUNT };
			if (const auto& countArg{ args.typegetv_any<opt::Flag, opt::Option>('l', "list") }; countArg.has_value()) {
				const auto& s{ countArg.value() };
				if (std::all_of(s.begin(), s.end(), str::stdpred::isdigit))
					count = str::stoi(s);
				else throw make_exception("Invalid List Count:  '", s, "' isn't a valid number!");
			}

			int i{ 0 };
			bool fst{ true };
			for (const auto& it : clipboard.history) {
				if (fst) fst = false;
				else {
					std::cout << '\n';
					if (!Config.quiet) std::cout << '\n';
				}

				if (!Config.quiet) std::cout << '[' << i << "]:\n";

				std::cout << it.getPreview(Config.preview_width, Config.preview_lines, !Config.quiet);

				if (++i >= count)
					break;
			}
		}
		// Show specific preview
		if (const auto& previewArg{ args.castgetv_any<size_t, opt::Flag, opt::Option>(str::stoull, 'p', "preview") }; previewArg.has_value()) {
			do_io_step = false;
			const auto& idx{ previewArg.value() };

			if (const auto& entry{ clipboard.history.get(idx) }; entry.has_value())
				std::cout << entry.value().getPreview(Config.preview_width, Config.preview_lines, !Config.quiet);
			else throw make_exception("Index ", idx, " does not exist in the history cache!");
		}
		// recall cache entry to clipboard
		if (const auto& index{ args.castgetv_any<size_t, opt::Flag, opt::Option>(str::stoull, 'r', "recall") }; index.has_value()) {
			do_io_step = false;
			const auto& idx{ index.value() };

			if (const auto& entry{ clipboard.history.get(idx) }; entry.has_value()) {
				// If the cache option was specified, cache the current clipboard data before overwriting it.
				if (autoCache || args.check_any<opt::Flag, opt::Option>('c', "cache")) {
					std::stringstream buffer;
					buffer << clipboard;
					clipboard.history.push(buffer.str());
				}
				entry.value().get() >> clipboard;
			}
			else throw make_exception("Index ", idx, " does not exist in the history cache!");
		}
		// add clipboard to cache (this has to occur AFTER recall or the indexes will be different)
		else if (autoCache || args.check_any<opt::Flag, opt::Option>('c', "cache")) {
			do_io_step = false;
			std::stringstream ss;
			ss << clipboard;
			clipboard.history.push(ss.str());
		}
		// Clear cached history
		if (args.checkopt("clear-cache")) {
			do_io_step = false;
			if (const auto& count{ clipboard.history.delete_all() }; count > 0)
				std::cout << term::get_msg() << "Deleted " << count << " cached clipboard entries." << std::endl;
			else throw make_exception("Failed to delete all cache entries!");
		}
		// get cache size
		if (args.check_any<opt::Flag, opt::Option>('S', "cache-size")) {
			do_io_step = false;
			std::cout << clipboard.history.size() << '\n';
		}


		// HANDLE PRIMARY I/O:

		bool hasPendingData{ hasPendingDataSTDIN() };
		const auto& setArgs{ args.typegetv_all<opt::Flag, opt::Option>('s', "set") };
		if (!setArgs.empty() || hasPendingData) {
			std::stringstream buffer;

			if (hasPendingData)
				buffer << std::cin.rdbuf();

			for (const auto& it : setArgs)
				buffer << it;

			if (buffer.tellp() != 0ull)
				buffer >> clipboard;
		}
		if ((do_io_step && (setArgs.empty() && !hasPendingData)) || args.checkflag('O'))
			std::cout << clipboard;

		return 0;
	} catch (const std::exception& ex) {
		std::cerr << term::get_fatal() << ex.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << term::get_fatal() << "An undefined exception occurred!" << std::endl;
		return 1;
	}
}
