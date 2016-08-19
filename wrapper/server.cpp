#include "wrap_communication.h"
#include "wrap_mmictrl.h"
#include "wrap_protocol.h"

#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <memory>

// TODO
#include "winsock2.h"

#include "windows.h"
#include <eckelmann/com_1st.h>
#define MMI
#include <eckelmann/com_def.h>
#include <eckelmann/com_sbx.h>

static bool g_run_server = true;

static void sigint_handler(int signal_number);
static void handle_message(wrap::message const &message);

int server_main(int argc, char **argv)
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

#if 0
	try {
		my_local_control ctrl("CNC1");
		std::map<std::uint16_t, double> params;

		for (int i = 0; i < 1024; i++) {
			params[i] = 0;
		}

		printf("init state: %d\n", ctrl.get_init_state());
		ctrl.load_firmware_blocked("C:\\Eckelmann\\StdHMI\\log\\download.cfg");
		ctrl.send_file_blocked("C:\\Eckelmann\\StdHMI\\log\\vom_nc.mk", "", wrap::transfer_block_type::MASCHINENKONSTANTEN);
		ctrl.send_file_blocked("C:\\Eckelmann\\StdHMI\\Prg\\KUGEL.DIN", "", wrap::transfer_block_type::NC_PROGRAMM);
		MSG_TR tr;
		tr.sb0_uc = SB0_AUFTRAG_KUC;
		tr.sb1_uc = SB1_PROGRAMM_START_KUC;
		tr.len_us = 8;
		tr.n.startprog_r.startmode_uc = 0x81; // 0x80 prognr in prognr_us, 0x1 normal start
		tr.n.startprog_r.prognr_us = 0x01;
		tr.n.startprog_r.satznr_us = 0x0;
		ctrl.send_message(&tr);
		ctrl.read_param_array(params);

		Sleep(2000);

		std::map<std::uint16_t, double> old_params = params;

		ctrl.read_param_array(params);

		for (int i = 0; i < 1024; i++) {
			if (std::fabs(old_params[i] - params[i]) > 0.000001) {
				printf("value %d changed: %f -> %f\n", i, old_params[i], params[i]);
			}
		}
	}
	catch (std::exception const &exception) {
		printf("Exception while working with CNC.\n");
		printf("%s\n", exception.what());
	}
#endif

	try {
		wrap::server server("10.0.0.138", 44455);

		std::signal(SIGINT, sigint_handler);

		server.set_message_callback(handle_message);

		while (g_run_server) {
			try {
				server.run_one(500);
			} catch (wrap::timeout_exception const &exception) {
				static_cast<void>(exception);
			}
		}
	} catch (std::exception const &exception) {
		std::fprintf(stderr, "Exception while running server.\n%s\n", exception.what());
		WSACleanup();
		return 1;
	}

	WSACleanup();

	return 0;
}

void sigint_handler(int signal_number)
{
	static_cast<void>(signal_number);

	std::printf("Stopping server executing after next polling timeout.\n");

	g_run_server = false;
}

void handle_message(wrap::message const &message)
{
	printf("received message %hu\n", message.type);
}

/*
void my_local_control::handle_message(wrap::callback_type_type type, unsigned long parameter)
{
	std::string type_string;

	switch (type) {
		case wrap::callback_type_type::MMI_DOWNLOAD_STATE:
			type_string = "Download State"; break;
		case wrap::callback_type_type::MMI_DOWNLOAD_PART:
			type_string = "Download Part"; break;
		case wrap::callback_type_type::MMI_DOWNLOAD_COMPLETE:
			type_string = "Download Complete"; break;
		case wrap::callback_type_type::MMI_DOWNLOAD_ERROR:
			type_string = "Download Error"; break;
		case wrap::callback_type_type::MMI_TRANSFER_STATE:
			type_string = "Transfer State"; break;
		case wrap::callback_type_type::MMI_TRANSFER_OK:
			type_string = "Transfer OK"; break;
		case wrap::callback_type_type::MMI_TRANSFER_ERROR:
			type_string = "Transfer Error"; break;
		case wrap::callback_type_type::MMI_TRANSFER_BREAK:
			type_string = "Transfer Break"; break;
		case wrap::callback_type_type::MMI_NCMSG_SENT:
			type_string = "NCMSG Sent"; break;
		case wrap::callback_type_type::MMI_NCMSG_NOT_SENT:
			type_string = "NCMSG Not Sent"; break;
		case wrap::callback_type_type::MMI_NCMSG_RECEIVED:
			type_string = "NCMSG Received"; break;
		case wrap::callback_type_type::MMI_ERROR_MSG:
			type_string = "Error MSG"; break;
		case wrap::callback_type_type::MMI_CYCLIC_CALL:
			type_string = "Cyclic Call"; break;
		case wrap::callback_type_type::MMI_DEFAPP_STATE:
			type_string = "DefApp State"; break;
		case wrap::callback_type_type::MMI_CAN_TRANSFER_STATE:
			type_string = "CAN Transfer State"; break;
		case wrap::callback_type_type::MMI_CAN_TRANSFER_COMPLETE:
			type_string = "CAN Transfer Complete"; break;
	}

	std::printf("Received CNC mssage: %s: %lu\n", type_string.c_str(), parameter);
};
*/
