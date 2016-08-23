#include "wrap_mmictrl_local.h"

#include <cassert>
#include <chrono>
#include <cstdio>
#include <thread>

namespace
{

void callback(std::uint8_t *data, wrap::mmictrl_local *ctrl);

bool g_run_mmictrl_local = true;

BOOL WINAPI ConsoleHandler(DWORD type);
int do_main(int argc, char **argv);

}

int main(int argc, char **argv)
{
	try {
		return do_main(argc, argv);
	} catch (std::exception const &exception) {
		std::fprintf(stderr, "Exception: %s\n", exception.what());
		return 1;
	}
}

namespace
{

BOOL WINAPI ConsoleHandler(DWORD type)
{
	switch (type) {
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
			std::printf("Interrupting console.\n");
			g_run_mmictrl_local = false;
			break;

		case CTRL_CLOSE_EVENT:
			std::printf("Closing console.\n");
			g_run_mmictrl_local = false;
			break;
	}

	return TRUE;
}

int do_main(int argc, char **argv)
{
	if (argc == 2 && std::string(argv[1]) == "/?") {
		std::printf("%s <name>\n", argv[0]);
		return 0;
	} else if (argc != 2) {
		std::printf("%s <name>\n", argv[0]);
		return 1;
	}

	const BOOL result = SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE);

	if (!result) {
        fprintf(stderr, "Unable to install handler.\n");
        return 1;
    }

	wrap::mmictrl_local ctrl;

	ctrl.set_message_callback([&ctrl](std::uint8_t *data) {
		callback(data, &ctrl);
	});

	ctrl.open(argv[1]);

	// get progname
	wrap::transfer_message msg;
	msg.controlblock0 = 3;
	msg.controlblock1 = 21;
	msg.data.resize(4);
	std::uint32_t prognr = 1;
	std::memcpy(msg.data.data(), &prognr, 4);;
	ctrl.send_message(msg);

	while (true) {
		bool state = ctrl.get_init_state();

		if (state) {
			break;
		}

		std::fprintf(stderr, "CNC not initialized yet.\n");
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	std::printf("CNC initialized.\n");
	//ctrl.load_firmware_blocked("C:\\Eckelmann\\StdHMI\\log\\download.cfg");

	std::map<std::uint16_t, double> parameters;

	parameters[0] = 0;

	while (g_run_mmictrl_local) {
		ctrl.read_param_array(parameters);

		for (auto const &parameter: parameters) {
			std::printf("P-Field [%d=%g]\n", parameter.first, parameter.second);
		}

		Sleep(1200);
	}

	return 0;
}

void callback(std::uint8_t *data, wrap::mmictrl_local *ctrl)
{
	wrap::callback_type_type cpp_type = static_cast<wrap::callback_type_type>(data[0]);
	std::uint32_t size = 0;

	size |= static_cast<std::uint32_t>(data[1]) << 24;
	size |= static_cast<std::uint32_t>(data[2]) << 16;
	size |= static_cast<std::uint32_t>(data[3]) << 8;
	size |= static_cast<std::uint32_t>(data[4]) << 0;

	const std::uint32_t payload_size = size - 5;

	wrap::transfer_message msg;

	if (cpp_type == wrap::callback_type_type::MMI_NCMSG_RECEIVED ||
	    cpp_type == wrap::callback_type_type::MMI_ERROR_MSG) {
		assert(payload_size >= 6);
		msg.controlblock0 = data[5];
		msg.controlblock1 = data[6];
		msg.controlblock2 = data[7];
		msg.current_block_number = data[8];
		msg.sender = data[9];
		msg.handle = data[10];
		msg.data.insert(msg.data.end(), data + 11, data + 11 + (payload_size - 6));
	}
 
	switch (cpp_type) {
		case wrap::callback_type_type::MMI_CYCLIC_CALL: {
			assert(payload_size == 0);
			std::printf("Cyclic CNC call.\n");
			break;
		}

		case wrap::callback_type_type::MMI_ERROR_MSG: {
			const std::uint8_t task = msg.data[0];
			const std::uint8_t cls = msg.data[1];
			std::int16_t num;
			std::memcpy(&num, msg.data.data() + 2, 2);
			const char *format = reinterpret_cast<char *>(msg.data.data() + 4);
			const char *str_data = reinterpret_cast<char *>(msg.data.data() + 84);
			char error_msg[1024];
			sprintf_s(error_msg, sizeof error_msg, format, str_data);
			std::printf("CNC ERROR: [task: %d, class: %d, num: %hd] [%s]\n", task, cls, num, error_msg);
		}

		case wrap::callback_type_type::MMI_NCMSG_RECEIVED: {
			if (msg.controlblock0 == 3 && msg.controlblock1 == 22) { // progname
				printf("recv size %d\n", msg.data.size());
				std::uint32_t prognr;
				std::memcpy(&prognr, msg.data.data(), 4);
				const std::string name(msg.data.begin() + 4, msg.data.end());
				std::printf("NCMSG: Program name for [num: %d]=[string: %s]\n", prognr, name.c_str());
			} else {
				std::printf("NCMSG: %d %d\n", msg.controlblock0, msg.controlblock1);
			}

			break;
		}

		case wrap::callback_type_type::MMI_UNIMPLEMENTED: {
			const std::uint32_t string_size = size - 1;
			const std::string string(data + 5, data + 5 + string_size);

			std::fprintf(stderr, "NCR message handling not implemented: %s\n", string.c_str());

			break;
		}

		default: {
			std::uint32_t parameter;
			std::memcpy(&parameter, data + 5, 4);
			std::fprintf(stderr, "NCR unknown message: %d [size=%d]\n", parameter, size);
		}
	}
}

}