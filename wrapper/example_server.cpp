#include "wrap_server.h"

#include <cassert>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>

namespace
{

void sigint_handler(int signal);

struct example_server
	: wrap::server
{
	example_server(std::string const &address, std::uint16_t port);

	void on_client_new(std::shared_ptr<client> const &client) override;
	void on_client_remove(std::shared_ptr<client> const &client) override;
	void on_client_message(std::shared_ptr<client> const &client,
	                       std::shared_ptr<wrap::message> const &message) override;

private:
	std::vector<std::shared_ptr<client>> clients_;
};

bool g_run_server = true;
std::unique_ptr<example_server> g_server;

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

	std::unique_ptr<example_server> example_server;
	example_server.reset(new struct example_server(argv[1], std::stoi(argv[2])));

	g_server = std::move(example_server);

	std::signal(SIGINT, sigint_handler);

	try {
		while (g_run_server) {
			g_server->run_one(500);
		}
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
	g_run_server = false;
}

example_server::example_server(std::string const &address, std::uint16_t port)
	: server(address, port)
{
}

void example_server::on_client_new(std::shared_ptr<client> const &client)
{
	clients_.push_back(client);
}

void example_server::on_client_remove(std::shared_ptr<client> const &client)
{
	for (auto it = clients_.begin(); it != clients_.end(); ++it) {
		if (*it == client) {
			clients_.erase(it);
			return;
		}
	}

	assert(false);
}

void example_server::on_client_message(std::shared_ptr<client> const &client,
                                       std::shared_ptr<wrap::message> const &message)
{
	std::printf("new client message: [id=%s, type=%d, size=%hu]\n",
	            client->address_string().c_str(), static_cast<int>(message->type), message->size);
}

}
