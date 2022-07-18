#include "clipboard.h"

#include <make_exception.hpp>

#include <sstream>
#ifdef OS_WIN
#include <Windows.h>
#endif

using namespace quip;

std::string quip::GetLastErrorMessage()
{
	DWORD err{ GetLastError() };
	LPSTR buf = 0;
	const auto& _{ FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, 2, buf, 256, NULL) };
	return buf == NULL ? std::string{} : std::string{ buf };
}

void quip::Clipboard::clear() const
{
	if (!OpenClipboard(NULL))
		return;
	EmptyClipboard();
	CloseClipboard();
}

void quip::Clipboard::set(const char* data) const
{
	const auto& len{ strlen(data) + 1 };

	// allocate global memory for data
	auto hMem{ GlobalAlloc(GMEM_MOVEABLE, len) };
	if (hMem == NULL)
		throw make_exception("Failed to allocate memory for incoming data!");
#	pragma warning(disable:C6387)
	memcpy(GlobalLock(hMem), data, len);
#	pragma warning(default:C6387)
	GlobalUnlock(hMem);

	if (!OpenClipboard(NULL))
		return;
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
}
void* quip::Clipboard::get(/*uint const& fmt = CF_TEXT*/) const
{
	HANDLE ret{ nullptr };
	if (!OpenClipboard(NULL))
		return ret;
	if (IsClipboardFormatAvailable(CF_TEXT))
		ret = GetClipboardData(CF_TEXT);
	CloseClipboard();
	return ret;
}


std::istream& quip::operator>>(std::istream& is, Clipboard& clipboard)
{
	std::stringstream ss;
	ss << is.rdbuf();
	const std::string s{ ss.str() };
	clipboard.set(s.c_str());
	return is;
}
std::ostream& quip::operator<<(std::ostream& os, const Clipboard& clipboard)
{
	if (HANDLE data{ clipboard.get(/*clipboard.defaultFormat.value_or(CF_TEXT)*/) }; data != nullptr) {
		const std::string str{ (char*)data };
		os << str;
	}
	return os;
}
