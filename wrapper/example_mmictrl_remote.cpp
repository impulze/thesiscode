#include "wrap_mmictrl_remote.h"

#include <csignal>
#include <cstdio>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

namespace
{

bool g_run_mmictrl_remote = true;
std::unique_ptr<wrap::mmictrl_remote> g_mmictrl_remote;

void sigint_handler(int signal);

}

int main(int argc, char **argv)
{
	if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "/?")) {
		std::printf("%s <name> <address> <port>\n", argv[0]);
		return 0;
	} else if (argc != 4) {
		std::fprintf(stderr, "%s <name> <address> <port>\n", argv[0]);
		return 1;
	}

	std::unique_ptr<wrap::mmictrl_remote> example_mmictrl_remote;

	try {
		example_mmictrl_remote.reset(new wrap::mmictrl_remote());

		example_mmictrl_remote->open(argv[1], argv[2], std::stoi(argv[3]));

		g_mmictrl_remote = std::move(example_mmictrl_remote);

		std::signal(SIGINT, sigint_handler);

		g_mmictrl_remote->load_firmware_blocked("C:\\Eckelmann\\StdHMI\\log\\download.cfg");
		std::map<std::uint16_t, double> parameters;
		parameters[0] = 0;
		while (g_run_mmictrl_remote) {
			g_mmictrl_remote->read_param_array(parameters);
			std::this_thread::sleep_for(std::chrono::milliseconds(1200));
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
	g_run_mmictrl_remote = false;
}

}
