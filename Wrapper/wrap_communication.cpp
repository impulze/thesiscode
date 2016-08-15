#include "wrap_communication.h"
#include "wrap_protocol.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <vector>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

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
	errno_error(std::string const &function);
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
static std::string error_string_from_function_call(std::string const &function);
static std::string address_to_string(const sockaddr_storage *address, int family);

struct client_data
{
	int socket;
	std::string address_string;
	std::vector<std::uint8_t> buffer;
};


namespace wrap
{

struct server_impl
{
	~server_impl();

	void remove_client(client_data &client_data);
	void new_client();
	void handle_client(client_data &client_data);
	client_data &lookup_client_by_socket(int socket);

	int socket;
	std::vector<client_data> clients;
	std::vector<pollfd> pollfds;
	std::function<void(message const &message)> callback;
};

struct client_impl
{
	void send_message(message const &message);

	int socket;
	pollfd pollfd;
};

}

getaddrinfo_error::getaddrinfo_error(int error)
	: std::runtime_error(error_string_from_getaddrinfo_error(error))
{
}

errno_error::errno_error(std::string const &function)
	: std::runtime_error(error_string_from_function_call(function))
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
	std::ostringstream strm;

	strm << "Error in 'getaddrinfo': ";
	strm << gai_strerror(error);

	return strm.str();
}

std::string error_string_from_function_call(std::string const &function)
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
	const char *buffer = strerror_r(errno, error_string, sizeof error_string);
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
			const char *buffer = inet_ntop(family, &(inet->sin_addr), temp_buffer, sizeof temp_buffer);

			if (buffer != NULL) {
				return buffer;
			}
		}

		case AF_INET6: {
			const sockaddr_in6 *inet = reinterpret_cast<const sockaddr_in6 *>(address);
			char temp_buffer[INET6_ADDRSTRLEN];
			const char *buffer = inet_ntop(family, &(inet->sin6_addr), temp_buffer, sizeof temp_buffer);

			if (buffer != NULL) {
				return buffer;
			}
		}

		default:
			throw std::runtime_error("Unexpected family while converting to string.");
	}


	throw errno_error("inet_ntop");
}

namespace wrap
{

server_impl::~server_impl()
{
	for (size_t i = 1; i < pollfds.size(); i++) {
		close(pollfds[i].fd);
	}
}

void server_impl::new_client()
{
	client_data client_data;

	sockaddr_storage storage;
	socklen_t storage_size = sizeof storage;

	client_data.socket = accept(socket, reinterpret_cast<sockaddr *>(&storage), &storage_size);

	if (client_data.socket == -1) {
		const std::string error_string = error_string_from_function_call("accept");
		fprintf(stderr, "%s\n", error_string.c_str());
		return;
	}

	try {
		client_data.address_string = address_to_string(&storage, storage.ss_family);
	} catch (std::exception const &exception) {
		close(socket);
		fprintf(stderr, "Error converting client address to string.\n");
		close(client_data.socket);
		return;
	}

	printf("New client connection from: <%s>\n", client_data.address_string.c_str());
	clients.push_back(client_data);

	pollfd entry;

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
	const size_t prev_size = client_data.buffer.size();

	client_data.buffer.resize(prev_size + READ_BUFFER);

	std::uint8_t *data = client_data.buffer.data();

	ssize_t result;

	result = read(client_data.socket, data + prev_size, READ_BUFFER);

	if (result <= 0) {
		if (result == 0) {
			printf("EOF while reading from client <%s>\n", client_data.address_string.c_str());
		} else {
			const std::string error_string = error_string_from_function_call("read");

			fprintf(stderr, "While reading from client <%s>:\n%s\n", client_data.address_string.c_str(), error_string.c_str());
		}

		remove_client(client_data);
		return;
	}

	const std::vector<message> messages = extract_messages_from_buffer(client_data.buffer);

	if (messages.size() == 0 && client_data.buffer.size() >= READ_BUFFER_MAX) {
		fprintf(stderr, "No messages received yet from client <%s> after <%d> bytes.\nForcing disconnect.\n", client_data.address_string.c_str(), READ_BUFFER_MAX);
		close(client_data.socket);
		remove_client(client_data);
	}

	for (size_t i = 0; i < messages.size(); i++) {
		if (callback) {
			callback(messages[i]);
		}
	}
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

	if (impl_->socket < 0) {
		delete impl_;
		throw errno_error("socket");
	}

	int result = bind(impl_->socket, reinterpret_cast<const sockaddr *>(&used_entry.address), used_entry.address_length);

	if (result != 0) {
		close(impl_->socket);
		delete impl_;
		throw errno_error("bind");
	}

	result = listen(impl_->socket, 5);

	if (result != 0) {
		close(impl_->socket);
		delete impl_;
		throw errno_error("listen");
	}

	std::string address_string;

	try {
		address_string = address_to_string(&used_entry.address, used_entry.family);
	} catch (std::exception const &exception) {
		close(impl_->socket);
		delete impl_;
		throw;
	}

	std::printf("Listening on <%s>:<%hu> for connections.\n", address_string.c_str(), port);
}

server::~server()
{
	close(impl_->socket);
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

	const int result = poll(impl_->pollfds.data(), impl_->pollfds.size(), timeout_ms);

	if (result < 0) {
		throw errno_error("poll");
	}

	if (result == 0) {
		throw timeout_exception("Timeout while waiting for incoming messages.");
	}

	size_t count = 0;

	for (size_t i = 0; i < impl_->pollfds.size() && count < result; i++) {
		const short revents = impl_->pollfds[i].revents;

		if (revents & POLLIN) {
			count++;

			if (i == 0) {
				impl_->new_client();
			} else {
				impl_->handle_client(impl_->lookup_client_by_socket(impl_->pollfds[i].fd));
			}
		} else if (revents & (POLLHUP|POLLERR|POLLNVAL)) {
			if (i == 0) {
				throw std::runtime_error("Unexpected polling state for server descriptor.");
			} else {
				throw std::runtime_error("Unexpected polling state for client descriptor.");
			}
		} else {
			assert(false);
		}
	}
}

void server::set_message_callback(std::function<void(message const &message)> callback)
{
	impl_->callback = callback;
}

server_impl::client &server_impl::lookup_client_by_socket(int socket)
{
	for (size_t i = 0; i < clients.size(); i++) {
		if (clients[i].socket == socket) {
			return clients[i];
		}
	}

	throw std::runtime_error("There's no client for that specific socket.");
}

void client_impl::send_message(message const &message)
{
	std::vector<std::uint8_t> bytes = message_to_bytes(message);

	ssize_t result = send(socket, bytes.data(), bytes.size(), 0);

	if (result != bytes.size()) {
		int old_errno = errno;
		fprintf(stderr, "Not all of the bytes sent to server.\n");
		close(socket);
		errno = old_errno;
		throw errno_error("send");
	}
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

	impl_->socket = socket(used_entry.family, used_entry.socket_type, used_entry.protocol);

	if (impl_->socket < 0) {
		delete impl_;
		throw errno_error("socket");
	}

	std::string address_string;

	try {
		address_string = address_to_string(&used_entry.address, used_entry.family);
	} catch (std::exception const &exception) {
		close(impl_->socket);
		delete impl_;
		throw;
	}

	printf("Initiate connection to <%s>:<%hu>\n", address_string.c_str(), port);

	const int result = connect(impl_->socket, reinterpret_cast<const sockaddr *>(&used_entry.address), used_entry.address_length);

	if (result != 0) {
		close(impl_->socket);
		delete impl_;
		throw errno_error("connect");
	}

	impl_->pollfd.fd = impl_->socket;
	impl_->pollfd.events = POLLIN;
	impl_->pollfd.revents = 0;
}

client::~client()
{
	close(impl_->socket);
	delete impl_;
}

message client::send_message(message const &message, int timeout_ms)
{
	try {
		impl_->send_message(message);
	} catch (std::exception const &exception) {
		throw;
	}

	const int result = poll(&(impl_->pollfd), 1, timeout_ms);

	if (result < 0) {
		throw errno_error("poll");
	}

	if (result == 0) {
		throw timeout_exception("Error during fetching message response.");
	}

	if (impl_->pollfd.revents & POLLIN) {
		...
	} else if (impl_->pollfd.revents & (POLLHUP|POLLERR|POLLNVAL)) {
		throw std::runtime_error("Unexpected polling state for client descriptor.");
	} else {
		assert(false);
	}
}

timeout_exception::timeout_exception(std::string const &error_msg)
	: std::runtime_error(error_msg)
{
}

}
