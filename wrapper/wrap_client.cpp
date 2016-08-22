#include "wrap_client.h"
#include "wrap_network.h"
#include "wrap_protocol.h"

#include <cassert>
#include <condition_variable>
#include <cstdio>
#include <mutex>
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
	void handle_server(std::shared_ptr<message> &message);

	std::string address_string;
	socket_type socket;
	std::vector<std::uint8_t> buffer;
	wrap::client *client;
	std::mutex mutex;
	std::condition_variable wakeup_condition;
	bool woken_up = false;
#ifndef WIN32
	int fake_fd[2];
#endif
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

	const int result = ::connect(socket, reinterpret_cast<const sockaddr *>(&used_entry.address),
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

void client::impl::handle_server(std::shared_ptr<message> &message)
{
	recv_bytes();

	while (!buffer.empty()) {
		auto const prev_size = buffer.size();

		message = wrap::message::from_bytes(buffer);

		if (buffer.size() == prev_size) {
			// needs more data
			break;
		} else if (!message) {
			throw std::runtime_error("Invalid message data received.");
		}
	}
}

client::client()
{
}

client::~client()
{
}

void client::connect(std::string const &address, std::uint16_t port)
{
#ifdef WIN32
	initialize_wsa();
#endif

	impl_.reset(new impl(address, port));
	impl_->client = this;

#ifndef WIN32
	const int result = pipe(impl_->fake_fd);

	if (result != 0) {
		std::fprintf(stderr, "pipe() failed: errno: %d\n", errno);
		throw std::runtime_error("pipe() failed");
	}

#else
#error "Unimplemented"
#endif
}

void client::interrupt()
{
#ifndef WIN32
	// avoid sending more data to fake fd
	std::unique_lock<std::mutex> lock(impl_->mutex);

	write(impl_->fake_fd[1], "", 1);
	impl_->wakeup_condition.wait(lock, [this](){ return impl_->woken_up; });
	impl_->woken_up = false;
#else
#error "Unimplemented"
#endif
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

std::shared_ptr<message> client::recv_message()
{
	if (!impl_) {
		throw std::runtime_error("Client not connected.");
	}

	std::shared_ptr<message> response;

	pollfd_type pollfd[2];
	pollfd[0].fd = impl_->socket;
	pollfd[0].events = POLLIN;
	pollfd[0].revents = 0;
	pollfd[1].fd = impl_->fake_fd[0];
	pollfd[1].events = POLLIN;
	pollfd[1].revents = 0;

	while (true) {
#ifndef WIN32
		const int result = poll(&pollfd[0], 2, 20000);
#else
		const int result = WSAPoll(&pollfd[0], 2, 20000);
#endif

#ifndef WIN32
printf("		poll returned: %d\n", result);
		if (result < 0) {
			if (errno == EINTR) {
				printf("poll() interrupted\n");
				return response;
			}
#else
		if (result == SOCKET_ERROR) {
#endif
			throw socket_error::create("poll");
		}

		if (result == 0) {
			throw std::runtime_error("Client timed out.");
		}

		if (pollfd[0].revents & POLLIN) {
			impl_->handle_server(response);

			if (response) {
				return response;
			}
		} else if (pollfd[1].revents & POLLIN) {
			std::unique_lock<std::mutex> lock(impl_->mutex);
			char buf;
			ssize_t res = read(pollfd[1].fd, &buf, 1);
			impl_->woken_up = true;
			impl_->wakeup_condition.notify_one();
			return response;
		} else {
			throw std::runtime_error("Unexpected polling state for client descriptors.");
		}
	}
}

}
