#ifndef MISC_H_INCLUDED
#define MISC_H_INCLUDED

#ifdef WIN32
#include <string>

#include <winsock2.h>
#include <windows.h>
#endif

#ifdef WIN32
std::string error_string_from_win32_error(DWORD win32_error);
#endif

#endif