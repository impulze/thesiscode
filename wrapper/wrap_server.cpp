#include "wrap_network.h"
#include "wrap_server.h"

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

struct server::client::impl
{
	socket_type socket;
	std::string address_string;
	std::vector<std::uint8_t> buffer;

	bool recv_bytes();
};

struct server::impl
{
	impl(std::string const &address, std::uint16_t port);
	~impl();

	void new_client();
	void remove_client(std::shared_ptr<client> const &client, bool call_callback = true);
	void handle_client(std::shared_ptr<client> const &client);
	void send_server_error(std::shared_ptr<client> const &client, std::string const &error_string);
	std::shared_ptr<client> lookup_client_by_socket(socket_type socket);

	std::vector<std::shared_ptr<client>> clients;
	socket_type socket;
	wrap::server *server;
};

server::impl::impl(std::string const &address, std::uint16_t port)
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

#ifndef WIN32
#error "Unimplemented"
#else
	int set_result;

	{
		int opt = 1;
		set_result = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
	}

	if (set_result == SOCKET_ERROR) {
		close_socket(socket);
		throw socket_error::create("setsockopt");
	}
#endif

	int result = bind(socket, reinterpret_cast<const sockaddr *>(&used_entry.address), used_entry.address_length);

	if (result != 0) {
		close_socket(socket);
		throw socket_error::create("bind");
	}

	result = listen(socket, 5);

	if (result != 0) {
		close_socket(socket);
		throw socket_error::create("listen");
	}

	std::string address_string;

	try {
		address_string = address_to_string(&used_entry.address, used_entry.family);
	} catch (std::exception const &) {
		close_socket(socket);
		throw;
	}

	std::printf("Listening on <%s>:<%hu> for connections.\n", address_string.c_str(), port);
}

server::impl::~impl()
{
	std::printf("Closing server.\n");
	close_socket(socket);
}

void server::impl::new_client()
{
	sockaddr_storage storage;
	socklen_t storage_size = sizeof storage;
	socket_type client_socket;

	client_socket = accept(socket, reinterpret_cast<sockaddr *>(&storage), &storage_size);

#ifndef WIN32
	if (socket < 0) {
#else
	if (socket == INVALID_SOCKET) {
#endif
		const std::string error_string = errno_error_string_from_function_call("accept", true);
		std::fprintf(stderr, "Error creating client socket.\n%s\n", error_string.c_str());
		return;
	}

	std::string address_string;

	try {
		address_string = address_to_string(&storage, storage.ss_family);
	}
	catch (std::exception const &) {
		close_socket(client_socket);
		throw;
	}

	std::printf("New client connection from: <%s>\n", address_string.c_str());

	std::shared_ptr<client> client;

	std::shared_ptr<server::client::impl> impl(new server::client::impl());
	impl->socket = client_socket;
	impl->address_string = address_string;

	client.reset(new server::client());
	client->impl_ = impl;
	clients.push_back(client);

	try {
		server->on_client_new(client);
	} catch (std::exception const &exception) {
		char exception_string[1024];
		snprintf(exception_string, sizeof exception_string,
		         "Exception while calling on_client_new: %s.",
		         exception.what());
		send_server_error(client, exception_string);
	}
}

void server::impl::remove_client(std::shared_ptr<client> const &client, bool call_callback)
{
	for (auto it = clients.begin(); it != clients.end(); ++it) {
		if (*it == client) {
			std::shared_ptr<server::client> temp_client = *it;
			clients.erase(it);
			std::printf("Removed client connection to: <%s>\n", temp_client->address_string().c_str());

			if (!call_callback) {
				break;
			}

			try {
				server->on_client_remove(temp_client);
			} catch (std::exception const &exception) {
				char exception_string[1024];
				snprintf(exception_string, sizeof exception_string,
				         "Exception while calling on_client_remove: %s.",
				         exception.what());
				send_server_error(temp_client, exception_string);
			}

			break;
		}
	}
}

void server::impl::handle_client(std::shared_ptr<client> const &client)
{
	bool result;

	try {
		result = client->impl_->recv_bytes();
	} catch (std::exception const &exception) {
		std::fprintf(stderr, "Client <%s> recv failed with: %s\nClient will be removed.\n",
		             client->impl_->address_string.c_str(), exception.what());
		remove_client(client);
		return;
	}

	if (!result) {
		std::fprintf(stderr, "Client <%s> has sent EOF. Client will be removed.\n",
		             client->impl_->address_string.c_str());
		remove_client(client);
		return;
	}

	while (!client->impl_->buffer.empty()) {
		auto const prev_size = client->impl_->buffer.size();

		std::shared_ptr<message> message = wrap::message::from_bytes(client->impl_->buffer);

		if (client->impl_->buffer.size() == prev_size) {
			// needs more data
			break;
		} else if (!message) {
			std::fprintf(stderr, "Client <%s> send invalid message data. Client will be removed.\n",
			             client->impl_->address_string.c_str());
			remove_client(client);
			return;
		} else {
			try {
				server->on_client_message(client, message);
			} catch (std::exception const &exception) {
				char exception_string[1024];
				snprintf(exception_string, sizeof exception_string,
				         "Exception while calling on_client_message: %s.",
				         exception.what());
				send_server_error(client, exception_string);
				return;
			}
		}
	}
}

void server::impl::send_server_error(std::shared_ptr<client> const &client, std::string const &error_string)
{
	std::fprintf(stderr, "The server encountered an error during client handling.\n%s\n", error_string.c_str());
	std::shared_ptr<message> response(new message(wrap::message_type::SERVER_ERROR));
	response->append(error_string);

	try {
		server->send_message_to_client(client, response);
	} catch (std::exception const &error) {
		std::fprintf(stderr, "Sending server error to client failed.\n%s\nClient will be removed.\n", error.what());
		
		for (auto it = clients.begin(); it != clients.end(); ++it) {
			if (*it == client) {
				std::shared_ptr<server::client> temp_client = *it;
				server->on_client_remove(client);
				clients.erase(it);
				return;
			}
		}
	}
}

std::shared_ptr<server::client> server::impl::lookup_client_by_socket(socket_type socket)
{
	for (auto &client: clients) {
		if (client->impl_->socket == socket) {
			return client;
		}
	}

	throw std::runtime_error("There's no client for that specific socket.");
}

server::server(std::string const &address, std::uint16_t port)
{
#ifdef WIN32
	wrap::initialize_wsa();
#endif

	impl_.reset(new impl(address, port));
	impl_->server = this;

}

server::~server()
{
}

bool server::run_one(int timeout_ms)
{
	if (!impl_) {
		throw std::runtime_error("Server not listening.");
	}

	std::vector<pollfd_type> pollfds;

	pollfd_type pollfd;
	pollfd.fd = impl_->socket;
	pollfd.events = POLLIN;
	pollfd.revents = 0;

	pollfds.push_back(pollfd);

	for (auto const &client: impl_->clients) {
		pollfd.fd = client->impl_->socket;
		pollfds.push_back(pollfd);
	}

#ifndef WIN32
	const int result = poll(pollfds.data(), pollfds.size(), timeout_ms);
#else
	const int result = WSAPoll(pollfds.data(), pollfds.size(), timeout_ms);
#endif

#ifndef WIN32
	if (result < 0) {
		if (errno == EINTR) {
			std::printf("poll() interrupted\n");
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

	int worked = 0;

	for (auto const &pollfd: pollfds) {
		std::shared_ptr<client> client;

		if (pollfd.fd != impl_->socket) {
			client = impl_->lookup_client_by_socket(pollfd.fd);
		}

		if (pollfd.revents & POLLIN) {
			if (pollfd.fd == impl_->socket) {
				impl_->new_client();
			} else {
				assert(client);
				impl_->handle_client(client);
			}

			worked++;
			continue;
		}

		if (pollfd.revents & (POLLERR|POLLHUP)) {
			if (pollfd.fd == impl_->socket) {
				throw std::runtime_error("Server socket invalid.");
			}

			std::fprintf(stderr, "Client <%s> socket invalid.\nClient will be removed.\n",
		             client->impl_->address_string.c_str());
			impl_->remove_client(client);

			worked++;
		}
	}

	if (worked != result) {
		throw std::runtime_error("Unexpected polling result.");
	}

	return true;
}

void server::send_message_to_client(std::shared_ptr<client> const &client,
                                    std::shared_ptr<message> const &message)
{
	if (!impl_) {
		throw std::runtime_error("Server not listening.");
	}

	std::vector<std::uint8_t> bytes;

	message->to_bytes(bytes);

#ifndef WIN32
	ssize_t result = send(client->impl_->socket, bytes.data(), bytes.size(), 0);
	const ssize_t requested = static_cast<ssize_t>(bytes.size());
#else
	int result = send(client->impl_->socket, reinterpret_cast<char *>(bytes.data()), bytes.size(), 0);
	const int requested = static_cast<int>(bytes.size());
#endif

	if (result != requested) {
		std::fprintf(stderr, "Not all of the bytes sent to client.\n");
		throw wrap::socket_error::create("send");
	}
}

void server::remove_client(std::shared_ptr<client> const &client)
{
	impl_->remove_client(client, false);
}

std::string const &server::client::address_string() const
{
	return impl_->address_string;
}

bool server::client::impl::recv_bytes()
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
			return false;
		} else {
			const std::string error_string = wrap::errno_error_string_from_function_call("read", true);
			char exception_string[1024];
			snprintf(exception_string, sizeof exception_string, "While reading from <%s>:\n%s\n",
			         address_string.c_str(), error_string.c_str());
			throw std::runtime_error(exception_string);
		}
	}

	buffer.insert(buffer.end(), read_buffer, read_buffer + result);

	return true;
}

}
