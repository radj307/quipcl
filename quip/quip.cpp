#include "rc/version.h"
#include "clipboard.h"

#include <ParamsAPI2.hpp>
#include <TermAPI.hpp>
#include <envpath.hpp>
#include <hasPendingDataSTDIN.h>

#include <iostream>

constexpr quip::uint DEFAULT_FORMAT{ 1 }; //< CF_TEXT == 1

struct Help {
	const std::string& programName;
	WINCONSTEXPR Help(const std::string& programName) :programName{ programName } {}
	friend std::ostream& operator<<(std::ostream& os, const Help& h)
	{
		return os
			<< "quip  v" << quip_VERSION_EXTENDED << '\n'
			<< "  CLI Clipboard Utility" << '\n'
			<< '\n'
			<< "USAGE:\n"
			<< "  " << h.programName << " [OPTIONS]" << '\n'
			<< '\n'
			<< "  This program is designed to be used with shell pipe operators." << '\n'
			<< "  # SETTING CLIPBOARD DATA" << '\n'
			<< '\n'
			<< "OPTIONS:\n"
			<< "  -h, --help           Shows this help display, then exits." << '\n'
			<< "  -v, --version        Prints the current version number, then exits." << '\n'
			<< "  -q, --quiet          Prevents non-essential console output." << '\n'
			;
	}
};

int main(const int argc, char** argv)
{
	try {
		std::ios_base::sync_with_stdio(false); //< disable cin <=> STDIO synchronization (disables buffering for cin)

		opt::ParamsAPI2 args{ argc, argv, 'f', "format" };
		const auto& [programPath, programName] { env::PATH().resolve_split(argv[0]) };

		bool quiet{ args.check_any<opt::Flag, opt::Option>('q', "quiet") };

		if (args.check_any<opt::Flag, opt::Option>('h', "help")) {
			std::cout << Help(programName.generic_string()) << std::endl;
			return 0;
		}
		else if (args.check_any<opt::Flag, opt::Option>('v', "version")) {
			if (!quiet)
				std::cout << "quip  v";
			std::cout << quip_VERSION_EXTENDED << std::endl;
			return 0;
		}

		// begin

		quip::Clipboard clipboard;

		if (hasPendingDataSTDIN()) { // input
			std::stringstream buffer;
			buffer << std::cin.rdbuf(); //< this works fine unless you're using Visual Studio's developer terminal; which inserts 12 bytes of garbage just for shits and giggles.
			buffer >> clipboard;
		}
		else // output
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
