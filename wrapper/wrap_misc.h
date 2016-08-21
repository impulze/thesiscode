#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED

#include <cstdint>
#include <stdexcept>
#include <string>

namespace wrap
{

struct errno_error
	: std::runtime_error
{
	static errno_error create(std::string const &function);

protected:
	errno_error(std::string const &error_string);
};

std::string error_string_from_win32_error(std::uint32_t win32_error);
std::string errno_error_string_from_function_call(std::string const &function, bool socket_related);
std::runtime_error exception_from_sprintf(size_t size, char const *format, ...);

#ifdef WIN32
void initialize_wsa();
#endif

}

#endif
