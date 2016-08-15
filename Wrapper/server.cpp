#include "wrap_communication.h"
#include "wrap_protocol.h"

#include <csignal>
#include <cstdio>

static bool g_run_server = true;

static void sigint_handler(int signal_number);
static void handle_message(wrap::message const &message);

int main(int argc, char **argv)
{
	try {
		wrap::server server("127.0.0.1", 55544);

		std::signal(SIGINT, sigint_handler);

		server.set_message_callback(handle_message);

		while (g_run_server) {
			server.run_one(2000);
		}
	} catch (std::exception const &exception) {
		std::printf("Exception while running server.\n%s\n", exception.what());
	}
}

static void sigint_handler(int signal_number)
{
	static_cast<void>(signal_number);

	std::printf("Stopping server executing after next polling timeout.\n");

	g_run_server = false;
}

static void handle_message(wrap::message const &message)
{
	printf("received message %hu\n", message.type);
	printf("string: %s\n", message.contents.data());
}
