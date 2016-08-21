#ifndef WRAP_SERVER_MMICTRL_H_INCLUDED
#define WRAP_SERVER_MMICTRL_H_INCLUDED

#include "wrap_server.h"

#include <memory>

namespace wrap
{

struct server_mmictrl
	: server
{
	server_mmictrl(std::string const &address, std::uint16_t port);

	void on_client_new(std::shared_ptr<client> const &client) override;
	void on_client_remove(std::shared_ptr<client> const &client) override;
	void on_client_message(std::shared_ptr<client> const &client,
	                       std::shared_ptr<message> const &message) override;

private:
	struct impl;
	std::shared_ptr<impl> impl_;
};

}

#endif
