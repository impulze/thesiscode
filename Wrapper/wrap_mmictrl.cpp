#include "wrap_communication.h"
#include "wrap_mmictrl.h"
#include "wrap_protocol.h"

#include <cassert>
#include <codecvt>
#include <cstdio>
#include <limits>
#include <locale>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "misc.h"

#ifdef WIN32
#include <windows.h>

#include "com_1st.h"
#define MMI
#include "com_def.h"
#define _export
#include "mmictrl.h"
#include "com_sbx.h"

#define MAKE_FUN_PTR_AND_VAR(name, ret, ...) \
typedef ret (WINAPI * name ## _fun_ptr)(##__VA_ARGS__); \
name ## _fun_ptr name ## _p;

#define LOAD_FUN(name, module) \
name ## _p = reinterpret_cast<name ## _fun_ptr>(GetProcAddress(module, #name));

#else
#include <arpa/inet.h> // hto* functions
#endif

#define MMIDBG(...) \
do {\
	SYSTEMTIME lt; \
	GetLocalTime(&lt); \
	std::printf("%04d/%02d/%02d %02d:%02d:%02d.%03d ", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds); \
	std::printf(__VA_ARGS__); \
	std::printf("\n"); \
	std::fflush(stdout); \
} while(0);

#ifdef WIN32
MAKE_FUN_PTR_AND_VAR(ncrOpenControl, HANDLE, LPCSTR, MSGCALLBACK, LPVOID)
MAKE_FUN_PTR_AND_VAR(ncrOpenDefaultControl, HANDLE, MSGCALLBACK, LPVOID)
MAKE_FUN_PTR_AND_VAR(ncrGetInitState, LONG, HANDLE)
MAKE_FUN_PTR_AND_VAR(ncrCloseControl, VOID, HANDLE)
MAKE_FUN_PTR_AND_VAR(ncrLoadFirmwareBlocked, LONG, HANDLE, LPCSTR)
MAKE_FUN_PTR_AND_VAR(ncrSendFileBlocked, LONG, HANDLE, LPCSTR, LPCSTR, LONG)
MAKE_FUN_PTR_AND_VAR(ncrSendMessage, BOOL, HANDLE, MSG_TR *)
MAKE_FUN_PTR_AND_VAR(ncrReadParamArray, BOOL, HANDLE, WORD *, double *, WORD)

static bool g_dll_loaded;

static std::map<LPVOID, wrap::control *> g_control_mapping;
static void WINAPI callback(ULONG type, ULONG param, LPVOID context);
static void load_cnc_dlls();
static void throw_transfer_exception(LONG result);
static LONG block_type_conversion(wrap::transfer_block_type type);
static wrap::callback_type_type conversion_callback_type_type(LONG type);
#endif

static void check_server_error(wrap::client &client, wrap::message const &response);
static void check_correct_response_type(wrap::client &client, wrap::message const &response, wrap::message_type type);

namespace wrap
{

#ifdef WIN32
struct local_control::local_impl
{
	HANDLE handle;
};
#endif

struct remote_control::remote_impl
{
	std::string name;
	std::shared_ptr<wrap::client> client;
};

}

namespace wrap
{

error::error(std::string const &message, std::uint32_t win32_error)
	: std::runtime_error(message),
	  win32_error(win32_error)
{
}

#ifdef WIN32
error error::create_error()
{
	const DWORD win32_error = GetLastError();
	const std::string error_string = error_string_from_win32_error(win32_error);

	return error(error_string, win32_error);
}
#endif

transfer_exception::transfer_exception(std::string const &message, transfer_status_type type)
	: std::runtime_error(message),
	  type(type)
{
}

transfer_exception transfer_exception::create(transfer_status_type type)
{
	switch (type) {
		case transfer_status_type::BREAK:
			return transfer_exception("Transfer cancelled by NC or MMI.", type);
		case transfer_status_type::TIMEOUT:
			return transfer_exception("Transfer timed out.", type);
		case transfer_status_type::NOINIT:
			return transfer_exception("Transer failed. Firmware is not loaded yet.", type);
		case transfer_status_type::QFULL:
			return transfer_exception("Transfer failed. Queue full.", type);
		default:
			assert(false);
	}

	throw std::runtime_error("Assertion.");
}

transfer_exception_error::transfer_exception_error(std::string const &message, transfer_status_type type, std::uint32_t win32_error)
	: error(message, win32_error),
	  transfer_exception(message, type),
	  std::runtime_error(message)
{
}

#ifdef WIN32
transfer_exception_error transfer_exception_error::create(transfer_status_type type)
{
	const DWORD win32_error = GetLastError();
	const std::string error_string = error_string_from_win32_error(win32_error);

	std::ostringstream strm;

	strm << "Transfer failed. ";
	strm << error_string;

	return transfer_exception_error(strm.str(), type, win32_error);
}
#endif

control::~control()
{
}

#ifdef WIN32
local_control::local_control(std::string const &name)
{
	if (!g_dll_loaded) {
		load_cnc_dlls();
		g_dll_loaded = true;
	}

	HANDLE handle = ncrOpenControl_p(name.c_str(), ::callback, this);

	if (handle == INVALID_HANDLE_VALUE) {
		throw error::create_error();
	}

	impl_ = new local_impl();
	impl_->handle = handle;

	try {
		g_control_mapping[this] = this;
	} catch (...) {
		ncrCloseControl_p(impl_->handle);
		delete impl_;
		throw;
	}
}

local_control::~local_control()
{
	ncrCloseControl_p(impl_->handle);

	delete impl_;

	for (std::map<LPVOID, control *>::iterator it = g_control_mapping.begin(); it != g_control_mapping.end();) {
		if (it->first == this) {
			g_control_mapping.erase(it);
			return;
		} else {
			++it;
		}
	}
}

bool local_control::get_init_state()
{
	const LONG result = ncrGetInitState_p(impl_->handle);

	if (result == -1) {
		throw std::runtime_error("Getting init state failed.");
	}

	return result == 1;
}

init_status local_control::load_firmware_blocked(std::string const &config_name)
{
	const LONG result = ncrLoadFirmwareBlocked_p(impl_->handle, config_name.c_str());

	if (result == 0) {
		return init_status::FIRMWARE_LOADED;
	} else if (result == -1) {
		return init_status::FIRMWARE_ALREADY_LOADED;
	}

	throw error::create_error();
}

void local_control::send_file_blocked(std::string const &name, std::string const &header,
                                transfer_block_type type)
{
	const LONG cnv_type = block_type_conversion(type);
	const LONG result = ncrSendFileBlocked_p(impl_->handle, name.c_str(), header.c_str(), cnv_type);

	throw_transfer_exception(result);
}

void local_control::send_message(MSG_TR *message)
{
	const BOOL result = ncrSendMessage_p(impl_->handle, message);

	if (result == TRUE) {
		return;
	}

	throw error::create_error();
}

void local_control::read_param_array(std::map<std::uint16_t, double> &parameters)
{
	if (parameters.size() > 0xFFFF) {
		throw std::runtime_error("Only 16-bit size allowed for read_param_array.");
	}

	std::vector<std::uint16_t> indices;
	std::vector<double> values;

	indices.reserve(parameters.size());
	values.reserve(parameters.size());

	for (auto const &parameter : parameters) {
		indices.push_back(parameter.first);
		values.push_back(parameter.second);
	}

	const BOOL result = ncrReadParamArray_p(impl_->handle, indices.data(), values.data(), static_cast<WORD>(parameters.size()));

	if (result != TRUE) {
		throw error::create_error();
	}

	std::vector<double>::const_iterator value_it = values.begin();
	std::map<std::uint16_t, double>::iterator parameters_it = parameters.begin();

	while (value_it != values.end()) {
		parameters_it->second = *value_it;
		++parameters_it;
		++value_it;
	}
}
#endif

remote_control::remote_control(std::string const &name, std::string const &address, std::uint16_t port)
{
	impl_ = new remote_impl;
	impl_->name = name;

	try {
		impl_->client.reset(new wrap::client(address, port));
	} catch (...) {
		delete impl_;
		throw;
	}

	try {
		wrap::message message(wrap::message_type::CTRL_OPEN);
		message.append(name);
		wrap::message response = impl_->client->send_message(message, 2000);

		check_server_error(*(impl_->client), response);
		check_correct_response_type(*(impl_->client), response, wrap::message_type::CTRL_OPEN_RESPONSE);

		if (response.contents[0] == 0) {
			std::printf("[STATUS] CNC Connection to <%s> successful.\n", name.c_str());
		} else {
			const std::string error_string = message.extract_string(1);
			char exception_string[1024];
			std::snprintf(exception_string, sizeof exception_string, "CNC Connection to <%s> failed.\n%s\n", name.c_str(), error_string.c_str());
			throw std::runtime_error(exception_string);
		}
	} catch (...) {
		delete impl_;
		throw;
	}
}

remote_control::~remote_control()
{
	try {
		wrap::message message(wrap::message_type::CTRL_CLOSE);
		wrap::message response = impl_->client->send_message(message, 2000);

		check_server_error(*(impl_->client), response);
		check_correct_response_type(*(impl_->client), response, wrap::message_type::CTRL_CLOSE_RESPONSE);

		if (response.contents[0] == 0) {
			std::printf("[STATUS] CNC Connection to <%s> closed.\n", impl_->name.c_str());
		} else {
			const std::string error_string = message.extract_string(1);
			std::fprintf(stderr, "CNC Closing connection for <%s> failed.\n%s\n", impl_->name.c_str(), error_string.c_str());
		}

		delete impl_;
	} catch (...) {
		delete impl_;
	}
}

bool remote_control::get_init_state()
{
	wrap::message message(wrap::message_type::CTRL_GET_INIT);
	wrap::message response = impl_->client->send_message(message, 1000);

	check_server_error(*(impl_->client), response);
	check_correct_response_type(*(impl_->client), response, wrap::message_type::CTRL_GET_INIT_RESPONSE);

	if (response.contents[0] == 0) {
		const bool state = (response.extract_bit8(0) == 1);
		return state;
	}

	const std::string error_string = message.extract_string(1);
	char exception_string[1024];
	std::snprintf(exception_string, sizeof exception_string, "CNC Getting state for <%s> failed.\n%s\n", impl_->name.c_str(), error_string.c_str());
	throw std::runtime_error(exception_string);
}

init_status remote_control::load_firmware_blocked(std::string const &config_name)
{
	wrap::message message(wrap::message_type::CTRL_LOAD_FIRMWARE_BLOCKED);
	message.append(config_name);
	wrap::message response = impl_->client->send_message(message, 20000);

	check_server_error(*(impl_->client), response);
	check_correct_response_type(*(impl_->client), response, wrap::message_type::CTRL_LOAD_FIRMWARE_BLOCKED_RESPONSE);

	if (response.contents[0] == 0) {
		return static_cast<init_status>(response.extract_bit8(1));
	}

	const std::string error_string = message.extract_string(1);
	const std::uint32_t win32_error = message.extract_bit32(1 + 2 + error_string.size());

	throw error(error_string, win32_error);
}

void remote_control::send_file_blocked(std::string const &name, std::string const &header,
                                       transfer_block_type type)
{
	wrap::message message(wrap::message_type::CTRL_SEND_FILE_BLOCKED);
	message.append(name);
	message.append(header);
	message.append(static_cast<std::uint8_t>(type));
	wrap::message response = impl_->client->send_message(message, 20000);

	check_server_error(*(impl_->client), response);
	check_correct_response_type(*(impl_->client), response, wrap::message_type::CTRL_SEND_FILE_BLOCKED_RESPONSE);

	if (response.contents[0] == 0) {
		return;
	}

	wrap::transfer_status_type status = static_cast<wrap::transfer_status_type>(message.extract_bit8(1));
	const std::string error_string = message.extract_string(2);

	if (status == wrap::transfer_status_type::FAIL) {
		const std::uint32_t win32_error = message.extract_bit32(2 + 2 + error_string.size());
		throw transfer_exception_error(error_string, status, win32_error);
	} else {
		throw transfer_exception(error_string, status);
	}
}

void remote_control::send_message(MSG_TR *message)
{
/*
	const BOOL result = ncrSendMessage_p(impl_->handle, message);

	if (result == TRUE) {
		return;
	}

	throw error::create_error();
*/
	throw;
}

void remote_control::read_param_array(std::map<std::uint16_t, double> &parameters)
{
	if (parameters.size() > 0xFFFF) {
		throw std::runtime_error("Only 16-bit size allowed for read_param_array.");
	}

/*
	std::vector<std::uint16_t> indices;
	std::vector<double> values;

	indices.reserve(parameters.size());
	values.reserve(parameters.size());

	for (auto const &parameter : parameters) {
		indices.push_back(parameter.first);
		values.push_back(parameter.second);
	}

	const BOOL result = ncrReadParamArray_p(impl_->handle, indices.data(), values.data(), static_cast<WORD>(parameters.size()));

	if (result != TRUE) {
		throw error::create_error();
	}

	std::vector<double>::const_iterator value_it = values.begin();
	std::map<std::uint16_t, double>::iterator parameters_it = parameters.begin();

	while (value_it != values.end()) {
		parameters_it->second = *value_it;
		++parameters_it;
		++value_it;
	}
*/
	throw;
}

}

#ifdef WIN32
void WINAPI callback(ULONG type, ULONG param, LPVOID context)
{
	for (auto &entry: g_control_mapping) {
		if (entry.first == context) {
			entry.second->handle_message(conversion_callback_type_type(type), param);
			return;
		}
	}

	assert(false);
}

void load_cnc_dlls()
{
	const HMODULE module = LoadLibrary(L"mmictrl.dll");

	if (module == NULL) {
		throw std::runtime_error("Unable to load mmictrl.dll");
	}

	LOAD_FUN(ncrOpenControl, module)
	LOAD_FUN(ncrOpenDefaultControl, module)
	LOAD_FUN(ncrGetInitState, module)
	LOAD_FUN(ncrCloseControl, module)
	LOAD_FUN(ncrLoadFirmwareBlocked, module)
	LOAD_FUN(ncrSendFileBlocked, module)
	LOAD_FUN(ncrSendMessage, module)
	LOAD_FUN(ncrReadParamArray, module)
}

void throw_transfer_exception(LONG result)
{
	switch (result) {
		case TRANSFER_OK:
			return;
		case TRANSFER_BREAK:
			throw wrap::transfer_exception::create(wrap::transfer_status_type::BREAK);
		case TRANSFER_TIMEOUT:
			throw wrap::transfer_exception::create(wrap::transfer_status_type::TIMEOUT);
		case TRANSFER_NOINIT:
			throw wrap::transfer_exception::create(wrap::transfer_status_type::NOINIT);
		case TRANSFER_QFULL:
			throw wrap::transfer_exception::create(wrap::transfer_status_type::QFULL);
		case TRANSFER_ERROR:
			throw wrap::transfer_exception_error::create(wrap::transfer_status_type::FAIL);
		default:
			assert(false);
	}
}

std::uint32_t block_type_conversion(wrap::transfer_block_type type)
{
	switch (type) {
		case wrap::transfer_block_type::NC_PROGRAMM:
			return SB1_NC_PROG_KUC;
		case wrap::transfer_block_type::ONLINE_PROGRAMM:
			return SB1_ONLINE_PROG_KUC;
		case wrap::transfer_block_type::WERKZEUG:
			return SB1_WERKZEUG_KORR_KUC;
		case wrap::transfer_block_type::WERKSTUECK:
			return SB1_WERKSTUECK_KORR_KUC;
		case wrap::transfer_block_type::ACHSE:
			return SB1_ACHS_KORR_KUC;
		case wrap::transfer_block_type::TECHNOLOGIE:
			return SB1_TECHNO_KUC;
		case wrap::transfer_block_type::MASCHINENKONSTANTEN:
			return SB1_MASCHINENKONSTANTEN_KUC;
		case wrap::transfer_block_type::KONFIGURATION:
			return SB1_KONFIG_KUC;
		case wrap::transfer_block_type::PFELD:
			return SB1_PFELD_BLOCK_KUC;
		case wrap::transfer_block_type::IO_MODUL_OFFSETS:
			return SB1_IOMODULOFFSET_KUC;
		case wrap::transfer_block_type::KORREKTUR_3D:
			return SB1_3D_KORREKTUR_KUC;
		case wrap::transfer_block_type::CAN_IO_OBJEKT:
			return SB1_CAN1_OBJECT_KUC;
		case wrap::transfer_block_type::CAN_DRIVE_OBJEKT:
			return SB1_CAN2_OBJECT_KUC;
		default:
			assert(false);
	}

	throw std::runtime_error("Assertion.");
}

wrap::callback_type_type conversion_callback_type_type(LONG type)
{
	switch (type) {
		case VK_MMI_DOWNLOAD_STATE:
			return wrap::callback_type_type::MMI_DOWNLOAD_STATE;
		case VK_MMI_DOWNLOAD_PART:
			return wrap::callback_type_type::MMI_DOWNLOAD_PART;
		case VK_MMI_DOWNLOAD_COMPLETE:
			return wrap::callback_type_type::MMI_DOWNLOAD_COMPLETE;
		case VK_MMI_DOWNLOAD_ERROR:
			return wrap::callback_type_type::MMI_DOWNLOAD_ERROR;
		case VK_MMI_TRANSFER_STATE:
			return wrap::callback_type_type::MMI_TRANSFER_STATE;
		case VK_MMI_TRANSFER_OK:
			return wrap::callback_type_type::MMI_TRANSFER_OK;
		case VK_MMI_TRANSFER_ERROR:
			return wrap::callback_type_type::MMI_TRANSFER_ERROR;
		case VK_MMI_TRANSFER_BREAK:
			return wrap::callback_type_type::MMI_TRANSFER_BREAK;
		case VK_MMI_NCMSG_SENT:
			return wrap::callback_type_type::MMI_NCMSG_SENT;
		case VK_MMI_NCMSG_NOT_SENT:
			return wrap::callback_type_type::MMI_NCMSG_NOT_SENT;
		case VK_MMI_NCMSG_RECEIVED:
			return wrap::callback_type_type::MMI_NCMSG_RECEIVED;
		case VK_MMI_ERROR_MSG:
			return wrap::callback_type_type::MMI_ERROR_MSG;
		case VK_MMI_CYCLIC_CALL:
			return wrap::callback_type_type::MMI_CYCLIC_CALL;
		case VK_MMI_DEFAPP_STATE:
			return wrap::callback_type_type::MMI_DEFAPP_STATE;
		case VK_MMI_CAN_TRANSFER_STATE:
			return wrap::callback_type_type::MMI_CAN_TRANSFER_STATE;
		case VK_MMI_CAN_TRANSFER_COMPLETE:
			return wrap::callback_type_type::MMI_CAN_TRANSFER_COMPLETE;
		default:
			assert(false);
	}

	throw std::runtime_error("Assertion.");
}

#endif

void check_server_error(wrap::client &client, wrap::message const &response)
{
	if (response.type == wrap::message_type::SERVER_ERROR) {
		const std::string error_string = response.extract_string(0);
		char exception_string[1024];
		std::snprintf(exception_string, sizeof exception_string, "A server error occured while handling client.\n%s", error_string.c_str());
		throw std::runtime_error(exception_string);
	}
}

void check_correct_response_type(wrap::client &client, wrap::message const &response, wrap::message_type type)
{
	if (response.type != type) {
		char exception_string[1024];
		std::snprintf(exception_string, sizeof exception_string, "Unexpected response type while handling client.\n");
		throw std::runtime_error(exception_string);
	}
}
