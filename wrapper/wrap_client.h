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
	client(std::string const &address, std::uint16_t port);
	virtual ~client();

	bool run_one(int timeout_ms);

protected:
	void send_message(std::shared_ptr<message> const &message);

private:
	virtual void on_message(std::shared_ptr<message> const &message) = 0;

private:
	struct impl;
	std::shared_ptr<impl> impl_;
};

}

#endif
