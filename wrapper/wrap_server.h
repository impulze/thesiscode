#ifndef WRAP_SERVER_H_INCLUDED
#define WRAP_SERVER_H_INCLUDED

#include <cstdint>
#include <memory>
#include <string>

#include "wrap_protocol.h"

namespace wrap
{

struct server
{
	struct client;

	server(std::string const &address, std::uint16_t port);
	virtual ~server();

	bool run_one(int timeout_ms);

protected:
	void send_message_to_client(std::shared_ptr<client> const &client,
	                            std::shared_ptr<message> const &message);
	void remove_client(std::shared_ptr<client> const &client);

private:
	virtual void on_client_new(std::shared_ptr<client> const &client) = 0;
	virtual void on_client_remove(std::shared_ptr<client> const &client) = 0;
	virtual void on_client_message(std::shared_ptr<client> const &client,
	                               std::shared_ptr<message> const &message) = 0;

private:
	struct impl;
	std::shared_ptr<impl> impl_;
};

struct server::client
{
	std::string const &address_string() const;

private:
	friend struct server;

	struct impl;
	std::shared_ptr<impl> impl_;
};

}

#endif
