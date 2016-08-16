#include "wrap_communication.h"
#include "wrap_mmictrl.h"
#include "wrap_protocol.h"

#if 0
#include <csignal>
#include <cstdio>
#include <cstring>

#ifndef WIN32
#include <unistd.h>
#else
#include "windows.h"
#endif

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

	for (auto const &message : messages) {
		client.send_message(message, 1000);
	}

#ifndef WIN32
	sleep(10);
#else
	Sleep(10);
#endif

#if 0
	std::signal(SIGINT, sigint_handler);

	while (g_run_server) {
		server.run_one(2000);
	}
#endif
}

#endif

struct my_remote_control
	: wrap::remote_control
{
	template <class... T>
	my_remote_control(T &&... args);

	void handle_message(wrap::callback_type_type type, unsigned long parameter) override;
};

int main(int argc, char **argv)
{
	my_remote_control ctrl("CNC1", "10.0.0.138", 44455);
}

template <class... T>
my_remote_control::my_remote_control(T &&... args)
	: wrap::remote_control(std::forward<T>(args)...)
{
}

void my_remote_control::handle_message(wrap::callback_type_type type, unsigned long parameter)
{
	std::printf("new message: %d %lu\n", static_cast<int>(type), parameter);
}
