#include "misc.h"

#include <cstdint>
#include <codecvt>
#include <locale>
#include <sstream>

#ifdef WIN32
std::string error_string_from_win32_error(DWORD win32_error)
{
	if (win32_error & (1 << 29)) {
		const std::uint8_t subsystem = static_cast<std::uint8_t>((win32_error >> 16) & 0xFF);
		const std::uint16_t error_number = static_cast<std::uint16_t>(win32_error & 0x7FFF);

		std::ostringstream strm;

		strm << "Error in Subsystem " << std::to_string(subsystem) << ". ";
		strm << "Error code " << std::to_string(error_number) << ".";

		return strm.str();
	}

	LPWSTR buffer = nullptr;
	size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, win32_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&buffer), 0, NULL);

	const std::wstring message(buffer, size);

	//Free the buffer.
	LocalFree(buffer);

	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;

	return converter.to_bytes(message);
}
#endif