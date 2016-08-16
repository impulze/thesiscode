#include "misc.h"
#include "wrap_communication.h"
#include "wrap_protocol.h"
#include "wrap_mmictrl.h"

#include <cassert>
#include <chrono>
#include <cerrno>
#include <codecvt>
#include <cstdio>
#include <cstring>
#include <locale>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#ifndef WIN32
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#define close_socket(fd) close(fd)
typedef int socket_type;
#else
#include "string.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#define close_socket(sock) closesocket(sock)
typedef SOCKET socket_type;
#endif

// incremental buffer size
#define READ_BUFFER 1024
// maximum buffer size at which point the client will be disconnected
#define READ_BUFFER_MAX 5000

struct getaddrinfo_error
	: std::runtime_error
{
	getaddrinfo_error(int error);
};

struct errno_error
	: std::runtime_error
{
	errno_error(std::string const &error_string);

	static errno_error create(std::string const &function);
};

struct socket_error
	: errno_error
{
	socket_error(std::string const &error_string);

	static socket_error create(std::string const &function);
};

struct lookup_entry
{
	int family;
	int socket_type;
	int protocol;
	sockaddr_storage address;
	socklen_t address_length;
};

static std::vector<lookup_entry> lookup(std::string const &address, std::uint16_t port);
static std::string error_string_from_getaddrinfo_error(int error);
static std::string error_string_from_function_call(std::string const &function, bool socket_related);
static std::string address_to_string(const sockaddr_storage *address, int family);

struct client_data
{
	std::unique_ptr<wrap::message> recv_message();
	void send_message(wrap::message const &message);

	socket_type socket;
	std::string address_string;
	std::vector<std::uint8_t> buffer;
	typedef std::vector<std::uint8_t>::size_type buffer_size_type;

#ifndef WIN32
	struct pollfd pollfd;
#else
	WSAPOLLFD pollfd;
#endif
};

#ifdef WIN32
struct client_local_control
	: wrap::local_control
{
	template <class... T>
	client_local_control(T &&... args);

	void handle_message(wrap::callback_type_type type, unsigned long parameter) override;
};

static std::map<std::uint16_t, std::unique_ptr<client_local_control>> g_controls;
#endif

namespace wrap
{

struct server_impl
{
	~server_impl();

	void remove_client(client_data &client_data);
	void new_client();
	void handle_client(client_data &client_data);
	client_data &lookup_client_by_socket(socket_type socket);

	socket_type socket;
	std::vector<client_data> clients;
	typedef std::vector<client_data>::size_type clients_size_type;

#ifndef WIN32
	std::vector<pollfd> pollfds;
	typedef std::vector<pollfd>::size_type pollfds_size_type;
#else
	std::vector<WSAPOLLFD> pollfds;
	typedef std::vector<WSAPOLLFD>::size_type pollfds_size_type;
#endif

	std::function<void(message const &message)> callback;
};

struct client_impl
{
	client_data data;
};

}

getaddrinfo_error::getaddrinfo_error(int error)
	: std::runtime_error(error_string_from_getaddrinfo_error(error))
{
}

errno_error::errno_error(std::string const &error_string)
	: std::runtime_error(error_string)
{
}

errno_error errno_error::create(std::string const &function)
{
	return errno_error(error_string_from_function_call(function, false));
}

socket_error::socket_error(std::string const &error_string)
	: errno_error(error_string)
{
}

socket_error socket_error::create(std::string const &function)
{
	return socket_error(error_string_from_function_call(function, true));
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
		throw getaddrinfo_error(status);
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

	return entries;
}

std::string error_string_from_getaddrinfo_error(int error)
{
	// Winsock uses Unicode gai_strerror
	std::wostringstream strm;

	strm << "Error in 'getaddrinfo': ";
	strm << gai_strerror(error);

	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	return converter.to_bytes(strm.str());
}

std::string error_string_from_function_call(std::string const &function, bool socket_related)
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


	throw socket_error("inet_ntop");
}

std::unique_ptr<wrap::message> client_data::recv_message()
{
	const size_t prev_size = buffer.size();

	buffer.resize(prev_size + READ_BUFFER);

	std::uint8_t *data = buffer.data();

#ifndef WIN32
	ssize_t result;

	result = recv(socket, data + prev_size, READ_BUFFER, 0);
#else
	int result;

	result = recv(socket, reinterpret_cast<char *>(data + prev_size), READ_BUFFER, 0);
#endif

	if (result <= 0) {
		char exception_string[1024];

		if (result == 0) {
			std::snprintf(exception_string, sizeof exception_string, "EOF while reading message from <%s>\n", address_string.c_str());
		} else {
			const std::string error_string = error_string_from_function_call("read", true);

			std::snprintf(exception_string, sizeof exception_string, "While reading from <%s>:\n%s\n", address_string.c_str(), error_string.c_str());
		}

		throw std::runtime_error(exception_string);
	}

	std::unique_ptr<wrap::message> message = wrap::message::from_bytes(buffer);

	if (!message) {
		if (buffer.size() >= READ_BUFFER_MAX) {
			char exception_string[1024];
			std::snprintf(exception_string, sizeof exception_string, "No messages received yet from client <%s> after <%d> bytes.\nForcing disconnect.\n", address_string.c_str(), READ_BUFFER_MAX);
			throw std::runtime_error(exception_string);
		}
	}

	return message;
}

void client_data::send_message(wrap::message const &message)
{
	std::vector<std::uint8_t> bytes;

	message.to_bytes(bytes);

#ifndef WIN32
	ssize_t result = send(socket, bytes.data(), bytes.size(), 0);
#else
	int result = send(socket, reinterpret_cast<char *>(bytes.data()), bytes.size(), 0);
#endif

	if (result != bytes.size()) {
		int old_errno = errno;
		std::fprintf(stderr, "Not all of the bytes sent to server.\n");
		close_socket(socket);
		errno = old_errno;
		throw socket_error("send");
	}
}

#ifdef WIN32
template <class... T>
client_local_control::client_local_control(T &&... args)
	: local_control(std::forward<T>(args)...)
{
}

void client_local_control::handle_message(wrap::callback_type_type type, unsigned long parameter)
{
}
#endif

namespace wrap
{

server_impl::~server_impl()
{
	for (size_t i = 1; i < pollfds.size(); i++) {
		close_socket(pollfds[i].fd);
	}
}

void server_impl::new_client()
{
	client_data client_data;

	sockaddr_storage storage;
	socklen_t storage_size = sizeof storage;

	client_data.socket = accept(socket, reinterpret_cast<sockaddr *>(&storage), &storage_size);

#ifndef WIN32
	if (client_data.socket < 0) {
#else
	if (client_data.socket == INVALID_SOCKET) {
#endif
		const std::string error_string = error_string_from_function_call("accept", true);
		std::fprintf(stderr, "Error creating client socket.\n%s\n", error_string.c_str());
		return;
	}

	try {
		client_data.address_string = address_to_string(&storage, storage.ss_family);
	}
	catch (std::exception const &exception) {
		static_cast<void>(exception);
		close_socket(socket);
		std::fprintf(stderr, "Error converting client address to string.\n");
		close_socket(client_data.socket);
		return;
	}

	std::printf("New client connection from: <%s>\n", client_data.address_string.c_str());
	clients.push_back(client_data);

#ifndef WIN32
	pollfd entry;
#else
	WSAPOLLFD entry;
#endif

	entry.fd = client_data.socket;
	entry.events = POLLIN;
	entry.revents = 0;

	pollfds.push_back(entry);
}

void server_impl::remove_client(client_data &client_data)
{
	for (size_t i = 0; i < clients.size(); i++) {
		if (clients[i].socket == client_data.socket) {
			clients.erase(clients.begin() + i);

			for (size_t j = 0; j < pollfds.size(); j++) {
				if (pollfds[j].fd == client_data.socket) {
					pollfds.erase(pollfds.begin() + j);
					break;
				}
			}

			break;
		}
	}
}

void server_impl::handle_client(client_data &client_data)
{
	std::unique_ptr<wrap::message> message;

	try {
		message = client_data.recv_message();
	} catch (std::exception const &exception) {
		close_socket(client_data.socket);
		remove_client(client_data);
		throw exception;
	}

	if (!message) {
		return;
	}

	if (callback) {
		callback(*message);
	}

	std::unique_ptr<wrap::message> response;

	try {
		switch (message->type) {
#ifdef WIN32
			case wrap::message_type::CTRL_OPEN: {
				static bool rand_initialized = false;

				if (!rand_initialized) {
					rand_initialized = true;
					std::srand(static_cast<unsigned>(std::time(NULL)));
				}

				std::uint16_t nlength;
				std::memcpy(&nlength, message->contents.data(), 2);
				const std::uint16_t length = ntohs(nlength);
				std::string name(message->contents.data() + 2, message->contents.data() + 2 + length);

				std::unique_ptr<client_local_control> new_control;

				const std::uint16_t max_id = static_cast<std::uint16_t>((std::numeric_limits<std::uint16_t>::max)());

				if (g_controls.size() == max_id) {
					// No more space TODO
				}

				response.reset(new wrap::message(wrap::message_type::CTRL_OPEN_RESPONSE));

				const std::uint16_t id = static_cast<std::uint16_t>(g_controls.size());

				try {
					new_control.reset(new client_local_control(name));
				} 	catch (std::exception const &exception) {
					std::fprintf(stderr, "Unable to open control to <%s>\n.%s\n", name.c_str(), exception.what());
					response->append(static_cast<std::uint8_t>(1));
					response->append(exception.what());
					break;
				}

				g_controls[id] = std::move(new_control);

				response->append(static_cast<std::uint8_t>(0));
				response->append(id);
				break;
			}
#endif

			default:
				response.reset(new wrap::message(wrap::message_type::OK));
				break;
		}
	} catch (std::exception const &exception) {
		response.reset(new wrap::message(wrap::message_type::SERVER_ERROR));
		response->append(exception.what());
	}

	client_data.send_message(*response);
}

server::server(std::string const &address, std::uint16_t port)
{
	impl_ = new server_impl();

	const std::vector<lookup_entry> entries = lookup(address, port);

	if (entries.size() == 0) {
		char *buffer = new char[address.size() + 200];
		const int result = std::snprintf(buffer, address.size() + 200, "No suitable socket addresses for <%s>:<%hu>\n", address.c_str(), port);
		const std::string buffer_cpp = buffer;
		delete buffer;

		if (result < 0) {
			delete impl_;
			throw errno_error("snprintf");
		}

		delete impl_;
		throw std::runtime_error(buffer_cpp);
	}

	const lookup_entry &used_entry = entries[0];

	impl_->socket = socket(used_entry.family, used_entry.socket_type, used_entry.protocol);

#ifndef WIN32
	if (impl_->socket < 0) {
#else
	if (impl_->socket == INVALID_SOCKET) {
#endif
		delete impl_;
		throw socket_error("socket");
	}

	int result = bind(impl_->socket, reinterpret_cast<const sockaddr *>(&used_entry.address), used_entry.address_length);

	if (result != 0) {
		close_socket(impl_->socket);
		delete impl_;
		throw socket_error("bind");
	}

	result = listen(impl_->socket, 5);

	if (result != 0) {
		close_socket(impl_->socket);
		delete impl_;
		throw socket_error("listen");
	}

	std::string address_string;

	try {
		address_string = address_to_string(&used_entry.address, used_entry.family);
	} catch (std::exception const &exception) {
		static_cast<void>(exception);
		close_socket(impl_->socket);
		delete impl_;
		throw;
	}

	std::printf("Listening on <%s>:<%hu> for connections.\n", address_string.c_str(), port);
}

server::~server()
{
	close_socket(impl_->socket);
	delete impl_;
}

void server::run_one(int timeout_ms)
{
	if (impl_->pollfds.size() < 1) {
		pollfd entry;

		entry.fd = impl_->socket;
		entry.events = POLLIN;
		entry.revents = 0;

		impl_->pollfds.push_back(entry);
	}

#ifndef WIN32
	const int result = poll(impl_->pollfds.data(), impl_->pollfds.size(), timeout_ms);
#else
	const int result = WSAPoll(impl_->pollfds.data(), impl_->pollfds.size(), timeout_ms);
#endif

#ifndef WIN32
	if (result < 0) {
#else
	if (result == SOCKET_ERROR) {
#endif
		throw socket_error("poll");
	}

	if (result == 0) {
		throw timeout_exception("Timeout while waiting for incoming messages.");
	}

	for (server_impl::pollfds_size_type i = 0; i < impl_->pollfds.size() && static_cast<server_impl::pollfds_size_type>(result); i++) {
		const short revents = impl_->pollfds[i].revents;

		if (revents & POLLIN) {
			if (i == 0) {
				impl_->new_client();
			} else {
				impl_->handle_client(impl_->lookup_client_by_socket(impl_->pollfds[i].fd));
			}
		} else if (revents & (POLLHUP|POLLERR|POLLNVAL)) {
			if (i == 0) {
				throw std::runtime_error("Unexpected polling state for server descriptor.");
			} else {
				client_data &client_data = impl_->lookup_client_by_socket(impl_->pollfds[i].fd);

				if (revents & POLLHUP) {
					std::printf("Client <%s> hang up, reading rest of channel.\n", client_data.address_string.c_str());
					impl_->handle_client(client_data);
				} else if (revents & POLLERR) {
					std::printf("Error condition at client <%s>.\n", client_data.address_string.c_str());
					close_socket(client_data.socket);
					impl_->remove_client(client_data);
				} else {
					throw std::runtime_error("Unexpected polling state for client descriptor..");
				}
			}
		}
	}
}

void server::set_message_callback(std::function<void(message const &message)> callback)
{
	impl_->callback = callback;
}

client_data &server_impl::lookup_client_by_socket(socket_type socket)
{
	for (size_t i = 0; i < clients.size(); i++) {
		if (clients[i].socket == socket) {
			return clients[i];
		}
	}

	throw std::runtime_error("There's no client for that specific socket.");
}

client::client(std::string const &address, std::uint16_t port)
{
	impl_ = new client_impl();

	const std::vector<lookup_entry> entries = lookup(address, port);

	if (entries.size() == 0) {
		char *buffer = new char[address.size() + 200];
		const int result = std::snprintf(buffer, address.size() + 200, "No suitable socket addresses for <%s>:<%hu>\n", address.c_str(), port);
		const std::string buffer_cpp = buffer;
		delete buffer;

		if (result < 0) {
			delete impl_;
			throw errno_error("snprintf");
		}

		delete impl_;
		throw std::runtime_error(buffer_cpp);
	}

	const lookup_entry &used_entry = entries[0];

	impl_->data.socket = socket(used_entry.family, used_entry.socket_type, used_entry.protocol);

#ifndef WIN32
	if (impl_->data.socket < 0) {
#else
	if (impl_->data.socket == INVALID_SOCKET) {
#endif
		delete impl_;
		throw socket_error("socket");
	}

	std::string address_string;

	try {
		address_string = address_to_string(&used_entry.address, used_entry.family);
	} catch (std::exception const &exception) {
		static_cast<void>(exception);
		close_socket(impl_->data.socket);
		delete impl_;
		throw;
	}

	printf("Initiate connection to <%s>:<%hu>\n", address_string.c_str(), port);

	const int result = connect(impl_->data.socket, reinterpret_cast<const sockaddr *>(&used_entry.address), used_entry.address_length);

	if (result != 0) {
		close_socket(impl_->data.socket);
		delete impl_;
		throw socket_error("connect");
	}

	impl_->data.pollfd.fd = impl_->data.socket;
	impl_->data.pollfd.events = POLLIN;
	impl_->data.pollfd.revents = 0;
}

client::~client()
{
	close_socket(impl_->data.socket);
	delete impl_;
}

message client::send_message(message const &message, int timeout_ms)
{
	impl_->data.send_message(message);

	const std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
	std::unique_ptr<wrap::message> response;

	while (true) {
#ifndef WIN32
		const int result = poll(&(impl_->data.pollfd), 1, timeout_ms);
#else
		const int result = WSAPoll(&(impl_->data.pollfd), 1, timeout_ms);
#endif

#ifndef WIN32
		if (result < 0) {
#else
		if (result == SOCKET_ERROR) {
#endif
			throw socket_error("poll");
		}

		if (result == 0) {
			throw timeout_exception("No server data for message response.");
		}

		if (impl_->data.pollfd.revents & POLLIN) {
			response = impl_->data.recv_message();

			if (!response) {
				const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
				const std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
				timeout_ms -= elapsed.count();

				if (timeout_ms <= 0) {
					throw timeout_exception("No response from server after timeout.");
				}
			} else {
				return *response;
			}
		} else if (impl_->data.pollfd.revents & (POLLHUP|POLLERR|POLLNVAL)) {
			throw std::runtime_error("Unexpected polling state for client descriptor.");
		} else {
			assert(false);
		}
	}
}

timeout_exception::timeout_exception(std::string const &error_msg)
	: std::runtime_error(error_msg)
{
}

}
