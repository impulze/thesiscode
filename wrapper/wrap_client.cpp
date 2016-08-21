#include "wrap_client.h"
#include "wrap_network.h"
#include "wrap_protocol.h"

#include <cassert>
#include <cstdio>
#include <vector>

#ifndef WIN32
#include <poll.h>
#include <unistd.h>
#define close_socket(fd) close(fd)
#define snprintf std::snprintf
typedef int socket_type;
typedef pollfd pollfd_type;
#else
#define close_socket(sock) closesocket(sock)
#define snprintf sprintf_s
typedef SOCKET socket_type;
typedef WSAPOLLFD pollfd_type;
#endif

#define READ_BUFFER_SIZE 1024

namespace wrap
{

struct client::impl
{
	impl(std::string const &address, std::uint16_t port);
	~impl();

	void recv_bytes();
	void handle_server();

	std::string address_string;
	socket_type socket;
	std::vector<std::uint8_t> buffer;
	wrap::client *client;
};

}

namespace wrap
{

client::impl::impl(std::string const &address, std::uint16_t port)
{
	const std::vector<lookup_entry> entries = lookup(address, port);
	const lookup_entry &used_entry = entries[0];

	socket = ::socket(used_entry.family, used_entry.socket_type, used_entry.protocol);

#ifndef WIN32
	if (socket < 0) {
#else
	if (socket == INVALID_SOCKET) {
#endif
		throw socket_error::create("socket");
	}

	try {
		address_string = address_to_string(&used_entry.address, used_entry.family);
	} catch (std::exception const &) {
		close_socket(socket);
		throw;
	}

	std::printf("Initiate connection to <%s>:<%hu>\n", address_string.c_str(), port);

	const int result = connect(socket, reinterpret_cast<const sockaddr *>(&used_entry.address),
	                           used_entry.address_length);

	if (result != 0) {
		close_socket(socket);
		throw socket_error::create("connect");
	}
}

client::impl::~impl()
{
	std::printf("Close client connection to %s.\n", address_string.c_str());
	close_socket(socket);
}

void client::impl::recv_bytes()
{
	char read_buffer[READ_BUFFER_SIZE];

#ifndef WIN32
	ssize_t result;
#else
	int result;
#endif

	result = recv(socket, read_buffer, sizeof read_buffer, 0);

	if (result <= 0) {
		if (result == 0) {
			throw std::runtime_error("Socket has sent EOF.");
		} else {
			throw socket_error::create("read");
		}
	}

	buffer.insert(buffer.end(), read_buffer, read_buffer + result);
}

void client::impl::handle_server()
{
	recv_bytes();

	while (!buffer.empty()) {
		auto const prev_size = buffer.size();

		std::shared_ptr<wrap::message> message = wrap::message::from_bytes(buffer);

		if (buffer.size() == prev_size) {
			// needs more data
			break;
		} else if (!message) {
			throw std::runtime_error("Invalid message data received.");
		} else {
			client->on_message(message);
		}
	}
}

client::client(std::string const &address, std::uint16_t port)
{
#ifdef WIN32
	initialize_wsa();
#endif

	impl_.reset(new impl(address, port));
	impl_->client = this;
}

client::~client()
{
}

bool client::run_one(int timeout_ms)
{
	if (!impl_) {
		throw std::runtime_error("Client not connected.");
	}

	pollfd_type pollfd;
	pollfd.fd = impl_->socket;
	pollfd.events = POLLIN;
	pollfd.revents = 0;

#ifndef WIN32
	const int result = poll(&pollfd, 1, timeout_ms);
#else
	const int result = WSAPoll(&pollfd, 1, timeout_ms);
#endif

#ifndef WIN32
	if (result < 0) {
		if (errno == EINTR) {
			printf("poll() interrupted\n");
			return true;
		}
#else
	if (result == SOCKET_ERROR) {
#endif
		throw socket_error::create("poll");
	}

	if (result == 0) {
		return false;
	}

	if (pollfd.revents & POLLIN) {
		impl_->handle_server();
	} else {
		throw std::runtime_error("Unexpected polling state for client descriptor.");
	}

	return true;
}

void client::send_message(std::shared_ptr<message> const &message)
{
	if (!impl_) {
		throw std::runtime_error("Client not connected.");
	}

	std::vector<std::uint8_t> bytes;

	message->to_bytes(bytes);

#ifndef WIN32
	ssize_t result = send(impl_->socket, bytes.data(), bytes.size(), 0);
	const ssize_t requested = static_cast<ssize_t>(bytes.size());
#else
	int result = send(impl_->socket, reinterpret_cast<char *>(bytes.data()), bytes.size(), 0);
	const int requested = static_cast<int>(bytes.size());
#endif

	if (result != requested) {
		std::fprintf(stderr, "Not all of the bytes sent to server.\n");
		throw wrap::socket_error::create("send");
	}
}

}