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

bool g_run_client = true;
std::unique_ptr<wrap::client> g_client;

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

	std::unique_ptr<wrap::client> example_client;
	example_client.reset(new wrap::client);

	g_client = std::move(example_client);

	g_client->connect(argv[1], std::stoi(argv[2]));

	std::signal(SIGINT, sigint_handler);

	try {
		const std::shared_ptr<wrap::message> ok_message(new wrap::message(wrap::message_type::OK));
		std::shared_ptr<wrap::message> response;

		while (g_run_client) {
			g_client->send_message(ok_message);
			auto response = g_client->recv_message();
			if (response) {
				std::printf("new message: [type=%d, size=%hu]\n",
					static_cast<int>(response->type), response->size);
			} else {
				std::printf("no response received, exiting");
				return 0;
			}
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

}
