#ifndef WRAP_CLIENT_H_INCLUDED
#define WRAP_CLIENT_H_INCLUDED

#include <cstdint>
#include <memory>
#include <string>

namespace wrap
{

struct message;

struct client
{
	client();
	virtual ~client();

	void connect(std::string const &address, std::uint16_t port);
	void interrupt();
	void send_message(std::shared_ptr<message> const &message);
	std::shared_ptr<message> recv_message();

	struct impl;
	std::shared_ptr<impl> impl_;
};

}

#endif
