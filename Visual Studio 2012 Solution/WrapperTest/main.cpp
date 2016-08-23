#include "wrap_server_mmictrl.h"

#include <cstdio>

#include <Windows.h>

namespace
{

bool g_run_server_mmictrl = true;

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
			g_run_server_mmictrl = false;
			break;

		case CTRL_CLOSE_EVENT:
			std::printf("Closing console.\n");
			g_run_server_mmictrl = false;
			break;
	}

	return TRUE;
}

int do_main(int argc, char **argv)
{
	if (argc == 2 && std::string(argv[1]) == "/?") {
		std::printf("%s <address> <port>\n", argv[0]);
		return 0;
	} else if (argc != 3) {
		std::printf("%s <address> <port>\n", argv[0]);
		return 1;
	}

	const BOOL result = SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE);

	if (!result) {
        fprintf(stderr, "Unable to install handler.\n");
        return 1;
    }

	wrap::server_mmictrl ctrl(argv[1], std::stoi(argv[2]));

	while (g_run_server_mmictrl) {
		ctrl.run_one(500);
	}

	return 0;
}

}