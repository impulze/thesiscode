#ifndef WRAP_COMMUNICATION_H_INCLUDED
#define WRAP_COMMUNICATION_H_INCLUDED

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>

namespace wrap
{

struct client_impl;
struct server_impl;
struct message;

struct timeout_exception
	: std::runtime_error
{
	timeout_exception(std::string const &error_msg);
};

struct server
{
	server(std::string const &address, std::uint16_t port);
	virtual ~server();

	void set_message_callback(std::function<void(message const &message)> callback);
	// can throw timeout_exception
	void run_one(int timeout_ms);

private:
	server_impl *impl_;
};

struct client
{
	client(std::string const &address, std::uint16_t port);
	virtual ~client();

	// can throw timeout_exception
	message send_message(message const &message, int timeout_ms);

private:
	client_impl *impl_;
};

}

#endif
