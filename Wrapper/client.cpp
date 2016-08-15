#include "wrap_communication.h"
#include "wrap_protocol.h"

#include <csignal>
#include <cstdio>
#include <cstring>

#include <unistd.h>

static bool g_run_server = true;

static void sigint_handler(int signal_number)
{
	static_cast<void>(signal_number);

	std::printf("Stopping server executing after next polling timeout.\n");

	g_run_server = false;
}

int main(int argc, char **argv)
{
	wrap::client client("127.0.0.1", 55544);

	std::vector<std::uint8_t> contents;

	{
		contents.resize(13);
		std::memcpy(contents.data(), "abcdefghijkl", 13);
	}

	std::vector<wrap::message> messages;

	messages.push_back(create_message(wrap::message_type::CHECK, contents));

	contents[11] = 'm';
	messages.push_back(create_message(wrap::message_type::CHECK, contents));

	contents[11] = 'n';
	messages.push_back(create_message(wrap::message_type::CHECK, contents));

	client.send_messages(messages);

	sleep(10);
#if 0
	std::signal(SIGINT, sigint_handler);

	while (g_run_server) {
		server.run_one(2000);
	}
#endif
}
