#include "wrap_mmictrl_remote.h"

#include <csignal>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#define FIRMWARE_FILE \
	"C:\\Eckelmann\\StdHMI\\log\\download.cfg"

namespace
{

bool g_run_mmictrl_remote = true;
std::unique_ptr<wrap::mmictrl_remote> g_mmictrl_remote;

void callback(std::uint8_t *data);
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
		example_mmictrl_remote->set_message_callback([](std::uint8_t *data) {
			callback(data);
		});
		example_mmictrl_remote->open(argv[1], argv[2], std::stoi(argv[3]));

		g_mmictrl_remote = std::move(example_mmictrl_remote);

		std::signal(SIGINT, sigint_handler);

		const bool state = g_mmictrl_remote->get_init_state();

		if (state) {
			std::printf("CNC already initialized\n");
		} else {
			const wrap::init_status init_status =
				g_mmictrl_remote->load_firmware_blocked(FIRMWARE_FILE);

			if (init_status == wrap::init_status::FIRMWARE_LOADED) {
				std::printf("CNC firmware loaded\n");

				const bool new_state = g_mmictrl_remote->get_init_state();

				if (new_state) {
					std::printf("CNC now initialized\n");
				} else {
					std::fprintf(stderr, "CNC firmware loaded but cannot be initialized.\n");
				}
			} else {
				std::fprintf(stderr, "CNC firmware was already loaded but cannot be initialized.\n");
				return 1;
			}
		}

#if 0
		g_mmictrl_remote->reset();
#endif

		std::map<std::uint16_t, double> parameters;
		parameters[0] = 0;

		while (g_run_mmictrl_remote) {
			if (parameters.size()) {
				g_mmictrl_remote->read_param_array(parameters);

				for (auto const &parameter: parameters) {
					std::printf("CNC P-Field [%d=%g]\n", parameter.first, parameter.second);
				}
			}

			g_mmictrl_remote->receive_file_blocked("0", wrap::transfer_block_type::NC_PROGRAMM);

			std::this_thread::sleep_for(std::chrono::milliseconds(3000));
		}


		{
			wrap::transfer_message msg;
			msg.controlblock0 = 3;
			msg.controlblock1 = 21;
			msg.data.resize(4);
			std::uint32_t prognr = 1;
			std::memcpy(msg.data.data(), &prognr, 4);;
			g_mmictrl_remote->send_message(msg);
		}

		return 0;
	} catch (std::exception const &exception) {
		std::fprintf(stderr, "%s\n", exception.what());
		return 1;
	}
}

namespace
{

void callback(std::uint8_t *data)
{
	std::uint32_t size = 0;
	size |= static_cast<std::uint32_t>(data[1]) << 24;
	size |= static_cast<std::uint32_t>(data[2]) << 16;
	size |= static_cast<std::uint32_t>(data[3]) << 8;
	size |= static_cast<std::uint32_t>(data[4]) << 0;

	const wrap::callback_type_type cpp_type = static_cast<wrap::callback_type_type>(data[0]);

	if (cpp_type == wrap::callback_type_type::MMI_NCMSG_RECEIVED) {
		wrap::transfer_message msg;
		msg.controlblock0 = data[5];
		msg.controlblock1 = data[6];
		msg.controlblock2 = data[7];
		msg.current_block_number = data[8];
		msg.sender = data[9];
		msg.handle = data[10];
		msg.data.insert(msg.data.end(), data + 11, data + 11 + (size - 11));

		if (msg.controlblock0 == 3 && msg.controlblock1 == 22) {
			std::uint32_t prognum;
			std::memcpy(&prognum, msg.data.data(), 4);
			const std::string name(msg.data.data() + 4, msg.data.data() + msg.data.size());
			std::printf("Progname response: [%d=%s]\n", prognum, name.c_str());
		} else {
			std::printf("NCMSG: [cb0=%d,cb1=%d]\n", msg.controlblock0, msg.controlblock1);
		}
	} else {
		std::printf("Callback: [type=%d]\n", data[0]);
	}
}

void sigint_handler(int signal)
{
	std::printf("Received CTRL+C, exiting...\n");
	g_run_mmictrl_remote = false;
}

}
