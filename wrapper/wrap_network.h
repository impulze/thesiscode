#ifndef WRAP_NETWORK_H_INCLUDED
#define WRAP_NETWORK_H_INCLUDED

#include "wrap_misc.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif

namespace wrap
{


struct timeout_exception
	: std::runtime_error
{
	timeout_exception(std::string const &error_msg);
};

struct lookup_error
	: std::runtime_error
{
	lookup_error(int error);
};

struct socket_error
	: errno_error
{
	static socket_error create(std::string const &function);

protected:
	socket_error(std::string const &error_msg);
};

struct lookup_entry
{
	int family;
	int socket_type;
	int protocol;
	sockaddr_storage address;
	socklen_t address_length;
};

std::vector<lookup_entry> lookup(std::string const &address, std::uint16_t port);
std::string address_to_string(const sockaddr_storage *address, int family);

}

#endif
