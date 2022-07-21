#include "Clipboard.h"

#include <make_exception.hpp>

#include <sstream>
#ifdef OS_WIN
#include <Windows.h>
#endif

using namespace quip;

void quip::Clipboard::set_raw(const char* data) const
{
#ifdef OS_WIN
	const auto& len{ strlen(data) + 1 };

	// allocate global memory for data
	auto hMem{ GlobalAlloc(GMEM_MOVEABLE, len) };
	if (hMem == NULL)
		throw make_exception("Failed to allocate memory for incoming data!");
	// copy data to global memory
#	pragma warning(disable:6387)
	memcpy(GlobalLock(hMem), data, len);
#	pragma warning(default:6387)
	GlobalUnlock(hMem);
	// open, empty, and set the clipboard data to the global memory, then close the clipboard
	if (!OpenClipboard(NULL))
		return;
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
#endif

	history.push(data);
}
std::string quip::Clipboard::get(bool const& throwOnInvalidFormat) const
{
#ifdef OS_WIN
	HANDLE ret{ nullptr };
	if (!OpenClipboard(NULL))
		return{};
	if (IsClipboardFormatAvailable(CF_TEXT))
		ret = GetClipboardData(CF_TEXT);
	else if (throwOnInvalidFormat)
		throw make_exception("Clipboard does not contain plaintext!");
	CloseClipboard();

	if (ret != nullptr)
		return{ (char*)ret };
#else
	if (const auto& latest{ history.get_latest() }; latest.has_value())
		return latest.value().str();
#endif
	return{};
}

void quip::Clipboard::clear() const
{
#ifdef OS_WIN
	if (!OpenClipboard(NULL))
		return;
	EmptyClipboard();
	CloseClipboard();
#else
	history.push();
#endif
}

template<var::Streamable... Ts>
void quip::Clipboard::set(Ts&&... data) const
{
	const std::string s{ str::stringify(std::forward<Ts>(data)...) };
	set_raw(s.c_str());
}

std::istream& quip::operator>>(std::istream& is, Clipboard& clipboard)
{
	std::stringstream ss;
	ss << is.rdbuf();
	const std::string s{ ss.str() };
	clipboard.set_raw(s.c_str());
	return is;
}
std::ostream& quip::operator<<(std::ostream& os, const Clipboard& clipboard)
{
	if (const auto data{ clipboard.get() }; !data.empty())
		os << data;
	return os;
}
