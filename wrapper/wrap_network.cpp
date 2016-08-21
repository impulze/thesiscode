#include "wrap_network.h"

#include <codecvt>
#include <locale>
#include <cstdio>
#include <cstring>
#include <sstream>

#ifndef WIN32
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#define snprintf std::snprintf
#else
#define snprintf sprintf_s
#endif

namespace
{

std::string error_string_from_getaddrinfo_error(int error);

}

namespace
{

std::string error_string_from_getaddrinfo_error(int error)
{
	// Winsock uses Unicode gai_strerror
	std::wostringstream strm;

	strm << "Error in 'getaddrinfo': ";
	strm << gai_strerror(error);

	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;

	return converter.to_bytes(strm.str());
}

}

namespace wrap
{

timeout_exception::timeout_exception(std::string const &error_msg)
	: std::runtime_error(error_msg)
{
}

std::vector<lookup_entry> lookup(std::string const &address, std::uint16_t port)
{
	addrinfo hints = addrinfo();

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	const char *node;

	if (address.size() == 0) {
		hints.ai_flags = AI_PASSIVE;
		node = NULL;
	} else {
		node = address.c_str();
	}

	const std::string port_string = std::to_string(port);
	const char *service = port_string.c_str();

	addrinfo *result;

	const int status = getaddrinfo(node, service, &hints, &result);

	if (status != 0) {
		throw lookup_error(status);
	}

	std::vector<lookup_entry> entries;

	for (addrinfo *it = result; it != NULL; it = it->ai_next) {
		lookup_entry entry;
		entry.family = it->ai_family;
		entry.socket_type = it->ai_socktype;
		entry.protocol = it->ai_protocol;
		std::memcpy(&entry.address, it->ai_addr, it->ai_addrlen);
		entry.address_length = it->ai_addrlen;

		entries.push_back(entry);

		const char *family_string;
		std::string address_string;

		try {
			address_string = address_to_string(&entry.address, entry.family);
		} catch (std::exception const &exception) {
			static_cast<void>(exception);
			freeaddrinfo(result);
			throw;
		}

		switch (entry.family) {
			case AF_INET:
				family_string = "IPv4";
				break;

			case AF_INET6:
				family_string = "IPv6";
				break;

			default:
				family_string = "Unknown";
				break;
		}

		std::printf("Looked up address for '%s': %s\n", family_string, address_string.c_str());
	}

	freeaddrinfo(result);

	if (entries.size() != 0) {
		return entries;
	}

	std::vector<char> error_msg_data;
	error_msg_data.resize(address.size() + 200);
	snprintf(error_msg_data.data(), error_msg_data.size(),
		"No suitable socket addresses for <%s>:<%hu>\n",
		address.c_str(), port);

	const std::string error_msg(error_msg_data.begin(), error_msg_data.end());

	if (result < 0) {
		throw errno_error::create("snprintf");
	}

	throw std::runtime_error(error_msg);
}

lookup_error::lookup_error(int error)
	: std::runtime_error(error_string_from_getaddrinfo_error(error))
{
}

socket_error::socket_error(std::string const &error_string)
	: errno_error(error_string)
{
}

socket_error socket_error::create(std::string const &function)
{
	return socket_error(errno_error_string_from_function_call(function, true));
}

std::string address_to_string(const sockaddr_storage *address, int family)
{
	switch (family) {
		case AF_INET: {
			const sockaddr_in *inet = reinterpret_cast<const sockaddr_in *>(address);
			char temp_buffer[INET_ADDRSTRLEN];
#ifndef WIN32
			const void *in_ptr = &(inet->sin_addr);
#else
			void *in_ptr = const_cast<IN_ADDR *>(&(inet->sin_addr));
#endif
			const char *buffer = inet_ntop(family, in_ptr, temp_buffer, sizeof temp_buffer);

			if (buffer != NULL) {
				return buffer;
			}
		}

		case AF_INET6: {
			const sockaddr_in6 *inet = reinterpret_cast<const sockaddr_in6 *>(address);
			char temp_buffer[INET6_ADDRSTRLEN];
#ifndef WIN32
			const void *in6_ptr = &(inet->sin6_addr);
#else
			void *in6_ptr = const_cast<IN6_ADDR *>(&(inet->sin6_addr));
#endif
			const char *buffer = inet_ntop(family, in6_ptr, temp_buffer, sizeof temp_buffer);

			if (buffer != NULL) {
				return buffer;
			}
		}

		default:
			throw std::runtime_error("Unexpected family while converting to string.");
	}


	throw wrap::socket_error::create("inet_ntop");
}

}
