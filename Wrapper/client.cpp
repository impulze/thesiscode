#include "wrap_communication.h"
#include "wrap_mmictrl.h"
#include "wrap_protocol.h"

#include <cmath>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

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

int client_main(int argc, char **argv)
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		fprintf(stderr, "WSAStartup failed with error: %d\n", err);
		return 1;
	}
#endif

	try {
		my_remote_control ctrl("CNC1", "10.0.0.138", 44455);
		printf("init state: %d\n", ctrl.get_init_state());
		printf("firmware update\n");
		ctrl.load_firmware_blocked("C:\\Eckelmann\\StdHMI\\log\\download.cfg");
		printf("send file\n");
		ctrl.send_file_blocked("C:\\Eckelmann\\StdHMI\\log\\vom_nc.mk", "", wrap::transfer_block_type::MASCHINENKONSTANTEN);
		printf("done\n");
		std::map<std::uint16_t, double> parameters;
		for (int i = 0; i < 1024; i++) {
			parameters[i] = 0;
		}
		ctrl.read_param_array(parameters);
		const std::map<std::uint16_t, double> old_parameters = parameters;
#ifndef WIN32
		sleep(10);
#else
		Sleep(10);
#endif
		ctrl.read_param_array(parameters);
		for (int i = 0; i < 1024; i++) {
			const double old_value = old_parameters.find(i)->second;
			const double new_value = parameters.find(i)->second;

			if (std::fabs(new_value - old_value) > 0.000001) {
				std::printf("param changed [%d]: %g -> %g\n", i, old_value, new_value);
			}
		}
	} catch (std::exception const &exception) {
		std::fprintf(stderr, "Exception while running client.\n%s\n", exception.what());
#ifdef WIN32
		WSACleanup();
#endif
		return 1;
	}

#ifdef WIN32
	WSACleanup();
#endif
	return 0;
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
