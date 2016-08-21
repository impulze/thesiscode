#include "wrap_misc.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <codecvt>
#include <locale>
#include <memory>
#include <sstream>
#include <string.h>

#ifdef WIN32
#include <Windows.h>
#define snprintf sprintf_s
#define vsnprintf(buf, s, fmt, va) ::vsnprintf_s(buf, s, _TRUNCATE, fmt, va)
#else
#define snprintf std::snprintf
#define vsnprintf std::vsnprintf
#endif

namespace
{

#ifdef WIN32
struct wsa_initializer
{
	wsa_initializer();
	~wsa_initializer();
};

std::unique_ptr<wsa_initializer> g_wsa_initializer;
#endif

}


namespace
{

#ifdef WIN32
wsa_initializer::wsa_initializer()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		char exception_string[1024];
		snprintf(exception_string, sizeof exception_string,
			"WSAStartup failed with error: %d\n", err);

		throw std::runtime_error(exception_string);
	}
}

wsa_initializer::~wsa_initializer()
{
	WSACleanup();
}
#endif

}

namespace wrap
{

errno_error::errno_error(std::string const &error_string)
	: std::runtime_error(error_string)
{
}

errno_error errno_error::create(std::string const &function)
{
	return errno_error(errno_error_string_from_function_call(function, false));
}

std::string error_string_from_win32_error(std::uint32_t win32_error)
{
	if (win32_error & (1 << 29)) {
		const std::uint8_t subsystem = static_cast<std::uint8_t>((win32_error >> 16) & 0xFF);
		const std::uint16_t error_number = static_cast<std::uint16_t>(win32_error & 0x7FFF);

		std::ostringstream strm;

		strm << "CNC DLL Error [subsystem:" << std::to_string(subsystem) << ",";
		strm << "number:" << std::to_string(error_number) << "]";

		return strm.str();
	}

#ifdef WIN32
	LPWSTR buffer = nullptr;
	size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, win32_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&buffer), 0, NULL);

	const std::wstring error_string(buffer, size);

	LocalFree(buffer);

	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;

	return converter.to_bytes(L"Win32 Error: " + error_string);
#else
	std::ostringstream strm;

	strm << "Win32 Error: " << win32_error;

	return strm.str();
#endif
}

std::string errno_error_string_from_function_call(std::string const &function, bool socket_related)
{
	char error_string[1024];

// The following code snippets is for POSIX compliant strerror variants, not the GNU ones
#if 0
	const int status = strerror_r(errno, error_string, sizeof error_string);

	if (status != 0) {
		std::ostringstream strm;

		strm << "strerror_r error: ";
		strm << errno;

		throw std::runtime_error(strm.str());
	} else {
		buffer = error_string;
	}
#else
#ifndef WIN32
	const char *buffer = strerror_r(errno, error_string, sizeof error_string);
#else
	if (socket_related) {
		const int error = WSAGetLastError();
		const std::string error_string_gen = error_string_from_win32_error(error);
		snprintf(error_string, sizeof error_string, error_string_gen.c_str());
	} else {
		static_cast<void>(strerror_s(error_string, sizeof error_string, errno));
	}

	const char *buffer = error_string;
#endif
#endif

	std::ostringstream strm;

	strm << "Error in '" << function << "': ";
	strm << buffer;

	return strm.str();
}

std::runtime_error exception_from_sprintf(size_t size, char const *format, ...)
{
	char *buffer = new char[size];
	std::string string_buffer;

	try {
		va_list argptr;
		va_start(argptr, format);
		vsnprintf(buffer, size, format, argptr);
		va_end(argptr);
		string_buffer = buffer;
	} catch (...) {
		delete[] buffer;
		throw;
	}

	delete[] buffer;

	return std::runtime_error(string_buffer);
}

#ifdef WIN32
void initialize_wsa()
{
	if (g_wsa_initializer) {
		return;
	}

	g_wsa_initializer.reset(new wsa_initializer);
}
#endif

}