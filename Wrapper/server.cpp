#include "wrap_cnc.h"
#include "wrap_communication.h"
#include "wrap_protocol.h"

#include <csignal>
#include <cstdio>

#include "winsock2.h"

#define MMIDBG(...) \
do {\
	SYSTEMTIME lt; \
	GetLocalTime(&lt); \
	std::printf("%04d/%02d/%02d %02d:%02d:%02d.%03d ", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds); \
	std::printf(__VA_ARGS__); \
	std::printf("\n"); \
	std::fflush(stdout); \
} while(0);

static bool g_run_server = true;

static void load_cnc_dlls();
static void sigint_handler(int signal_number);
static void handle_message(wrap::message const &message);

int main(int argc, char **argv)
{
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

	try {
		wrap::server server("127.0.0.1", 55544);

		std::signal(SIGINT, sigint_handler);

		server.set_message_callback(handle_message);

		while (g_run_server) {
			try {
				server.run_one(2000);
			} catch (wrap::timeout_exception const &exception) {
				static_cast<void>(exception);
			}
		}
	} catch (std::exception const &exception) {
		std::printf("Exception while running server.\n%s\n", exception.what());
	}

	WSACleanup();
}


static void load_cnc_dlls()
{
	const HMODULE module = LoadLibrary(L"mmictrl.dll");

	if (module == NULL) {
		throw std::runtime_error("Unable to load mmictrl.dll");
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
