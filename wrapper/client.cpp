#include "wrap_communication.h"
#include "wrap_mmictrl.h"
#include "wrap_protocol.h"

#include <cmath>
#include <cstring>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
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
		wrap::remote_control ctrl;
		ctrl.open("CNC1", "10.0.0.138", 44455);
		printf("init: %d\n", ctrl.get_init_state());
		wrap::init_status status = ctrl.load_firmware_blocked("C:\\Eckelmann\\StdHMI\\log\\download.cfg");
		printf("firmware update: %d\n", status);
		ctrl.send_file_blocked("C:\\Eckelmann\\StdHMI\\log\\vom_nc.mk", "", wrap::transfer_block_type::MASCHINENKONSTANTEN);
		printf("machine constants sent\n");

		wrap::transfer_message msg = wrap::transfer_message();

		// reset
		msg.controlblock0 = 85;
		msg.controlblock1 = 170;

#if 0
		// get progname
		msg.controlblock0 = 3;
		msg.controlblock1 = 21;
		msg.data.resize(4);
		std::uint32_t prognr = 1;
		std::memcpy(msg.data.data(), &prognr, 4);
		std::uint32_t prognum = htonl(0);
		std::memcpy(msg.data.data(), &prognum, 4);
#endif

		ctrl.send_message(msg);

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