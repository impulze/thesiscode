#include "wrap_client.h"
#include "wrap_protocol.h"

#include <csignal>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>

namespace
{

void sigint_handler(int signal);

struct example_client
	: wrap::client
{
	example_client(std::string const &address, std::uint16_t port);

	void send_message(std::shared_ptr<wrap::message> const &message);

	void on_message(std::shared_ptr<wrap::message> const &message) override;
};

bool g_run_client = true;
std::unique_ptr<example_client> g_client;

}

int main(int argc, char **argv)
{
	if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "/?")) {
		std::printf("%s <address> <port>\n", argv[0]);
		return 0;
	} else if (argc != 3) {
		std::fprintf(stderr, "%s <address> <port>\n", argv[0]);
		return 1;
	}

	std::unique_ptr<example_client> example_client;
	example_client.reset(new struct example_client(argv[1], std::stoi(argv[2])));

	g_client = std::move(example_client);

	std::signal(SIGINT, sigint_handler);

	try {
		while (g_run_client) {
			std::shared_ptr<wrap::message> message(new wrap::message(wrap::message_type::OK));
			g_client->send_message(message);
			g_client->run_one(1000);
		}

		return 0;
	} catch (std::exception const &exception) {
		std::fprintf(stderr, "%s\n", exception.what());
		return 1;
	}
}

namespace
{

void sigint_handler(int signal)
{
	std::printf("Received CTRL+C, exiting...\n");
	g_run_client = false;
}

example_client::example_client(std::string const &address, std::uint16_t port)
	: client(address, port)
{
}

void example_client::send_message(std::shared_ptr<wrap::message> const &message)
{
	return client::send_message(message);
}

void example_client::on_message(std::shared_ptr<wrap::message> const &message)
{
	std::printf("new message: [type=%d, size=%hu]\n",
	            static_cast<int>(message->type), message->size);
}

}
