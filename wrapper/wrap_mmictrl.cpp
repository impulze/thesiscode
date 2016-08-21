#include "wrap_communication.h"
#include "wrap_mmictrl.h"
#include "wrap_protocol.h"

#include <cassert>
#include <codecvt>
#include <cstdio>
#include <cstring>
#include <limits>
#include <locale>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "misc.h"

#ifdef WIN32
#include <windows.h>

#define snprintf sprintf_s

#include <eckelmann/com_1st.h>
#define MMI
#include <eckelmann/com_def.h>
#define _export
#include <eckelmann/mmictrl.h>
#include <eckelmann/com_sbx.h>

#define MAKE_FUN_PTR_AND_VAR(name, ret, ...) \
typedef ret (WINAPI * name ## _fun_ptr)(##__VA_ARGS__); \
name ## _fun_ptr name ## _p;

#define LOAD_FUN(name, module) \
name ## _p = reinterpret_cast<name ## _fun_ptr>(GetProcAddress(module, #name));

#else
#include <arpa/inet.h> // hto* functions

#define snprintf std::snprintf

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

namespace
{

bool g_dll_loaded;
std::vector<wrap::control *> g_controls;

void WINAPI callback(ULONG type, ULONG param, LPVOID context);
void load_cnc_dlls();
void throw_transfer_exception(LONG result);
LONG block_type_conversion(wrap::transfer_block_type type);
wrap::callback_type_type conversion_callback_type_type(LONG type);
void eckelmann_cpp_to_msg(wrap::transfer_message const &message, MSG_TR *em_message);

}
#endif

namespace
{

void check_server_error(wrap::client &client, wrap::message const &response);
void check_correct_response_type(wrap::client &client, wrap::message const &response, wrap::message_type type);
void eckelmann_msg_to_cpp(unsigned long param, wrap::transfer_message const &message);

}

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
local_control::local_control()
{
	impl_ = new local_impl();
	impl_->handle = NULL;

	try {
		g_controls.push_back(this);
	} catch (...) {
		delete impl_;
		throw;
	}
}

local_control::~local_control()
{
	if (impl_->handle != NULL) {
		ncrCloseControl_p(impl_->handle);
	}

	delete impl_;

	for (auto it = g_controls.begin(); it != g_controls.end();) {
		if (*it == this) {
			g_controls.erase(it);
			return;
		} else {
			++it;
		}
	}
}

void local_control::open(std::string const &name)
{
	if (!g_dll_loaded) {
		load_cnc_dlls();
		g_dll_loaded = true;
	}

	impl_->handle = ncrOpenControl_p(name.c_str(), ::callback, this);

	if (impl_->handle == INVALID_HANDLE_VALUE) {
		impl_->handle = NULL;
		throw error::create_error();
	}
}

bool local_control::get_init_state()
{
	if (impl_->handle == NULL) {
		throw std::runtime_error("Control not opened.");
	}

	const LONG result = ncrGetInitState_p(impl_->handle);

	if (result == -1) {
		throw std::runtime_error("Getting init state failed.");
	}

	return result == 1;
}

init_status local_control::load_firmware_blocked(std::string const &config_name)
{
	if (impl_->handle == NULL) {
		throw std::runtime_error("Control not opened.");
	}

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
	if (impl_->handle == NULL) {
		throw std::runtime_error("Control not opened.");
	}

	const LONG cnv_type = block_type_conversion(type);
	const LONG result = ncrSendFileBlocked_p(impl_->handle, name.c_str(), header.c_str(), cnv_type);

	throw_transfer_exception(result);
}

void local_control::send_message(transfer_message const &message)
{
	if (impl_->handle == NULL) {
		throw std::runtime_error("Control not opened.");
	}

	MSG_TR em_message;

	em_message.sb0_uc = message.controlblock0;
	em_message.sb1_uc = message.controlblock1;
	em_message.sb2_uc = message.controlblock2;
	em_message.handle_uc = message.handle;
	em_message.index_uc = message.current_block_number;
	em_message.len_us = message.data.size();
	em_message.modul_uc = message.sender;

	for (decltype(message.data.size()) i = 0; i < message.data.size(); i++) {
		em_message.n.val_auc[i] = message.data[i];
	}

	const BOOL result = ncrSendMessage_p(impl_->handle, &em_message);

	if (result == TRUE) {
		return;
	}

	throw error::create_error();
}

void local_control::read_param_array(std::map<std::uint16_t, double> &parameters)
{
	if (impl_->handle == NULL) {
		throw std::runtime_error("Control not opened.");
	}

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
	std::vector<std::uint16_t>::const_iterator index_it = indices.begin();

	while (value_it != values.end()) {
		parameters[*index_it] = *value_it;
		++index_it;
		++value_it;
	}
}

void local_control::on_message(callback_type_type type, void *parameter)
{
	assert(impl_->handle);

	switch (type) {
		case wrap::callback_type_type::MMI_CYCLIC_CALL:
			break; // don't print cyclic call messages

		case wrap::callback_type_type::MMI_ERROR_MSG: {
			transfer_message *msg = static_cast<transfer_message *>(parameter);
			const std::uint8_t task = msg->data[0];
			const std::uint8_t cls = msg->data[1];
			std::int16_t num;
			std::memcpy(&num, msg->data.data() + 2, 2);
			const char *format = reinterpret_cast<char *>(msg->data.data() + 4);
			const char *data = reinterpret_cast<char *>(msg->data.data() + 84);
			char error_msg[1024];
			snprintf(error_msg, sizeof error_msg, format, data);
			std::printf("Received local NC ERROR: [task: %d, class: %d, num: %hd] [%s]\n", task, cls, num, error_msg);
			break;
		}

		case wrap::callback_type_type::MMI_NCMSG_RECEIVED: {
			transfer_message *msg = static_cast<transfer_message *>(parameter);
			if (msg->controlblock0 == 3 && msg->controlblock1 == 22) { // progname
				std::uint32_t prognr;
				std::memcpy(&prognr, msg->data.data(), 4);
				const std::string name(msg->data.begin() + 4, msg->data.end());
				std::printf("Received local NCMSG: Program name for [num: %d]=[string: %s]\n", prognr, name.c_str());
			} else {
				std::printf("Received local NCMSG: %d %lu\n", msg->controlblock0, msg->controlblock1);
			}
			break;
		}

		case wrap::callback_type_type::MMI_UNIMPLEMENTED: {
			const char *msg = static_cast<const char *>(parameter);
			std::fprintf(stderr, "NCR message handling not implemented: %s\n", msg);
			break;
		}

		default: {
			const unsigned long parameter_long = *static_cast<unsigned long *>(parameter);
			std::printf("Received local DLL message: %d %lu\n", type, parameter_long);
			break;
		}
	}
}
#endif

remote_control::remote_control()
{
	impl_ = new remote_impl;
}

remote_control::~remote_control()
{
	if (!impl_->client) {
		delete impl_;
		return;
	}

	try {
		wrap::message message(wrap::message_type::CTRL_CLOSE);
		wrap::message response = impl_->client->send_message(message, 2000);

		check_server_error(*(impl_->client), response);
		check_correct_response_type(*(impl_->client), response, wrap::message_type::CTRL_CLOSE_RESPONSE);

		if (response.contents[0] == 0) {
			std::printf("[STATUS] CNC Connection to <%s> closed.\n", impl_->name.c_str());
		} else {
			const std::string error_string = response.extract_string(1);
			std::fprintf(stderr, "CNC Closing connection for <%s> failed.\n%s\n", impl_->name.c_str(), error_string.c_str());
		}

		delete impl_;
	} catch (...) {
		delete impl_;
	}
}

void remote_control::open(std::string const &name, std::string const &address, std::uint16_t port)
{
	impl_->name = name;
	impl_->client.reset(new wrap::client(address, port));

	try {
		wrap::message message(wrap::message_type::CTRL_OPEN);
		message.append(name);
		wrap::message response = impl_->client->send_message(message, 2000);

		check_server_error(*(impl_->client), response);
		check_correct_response_type(*(impl_->client), response, wrap::message_type::CTRL_OPEN_RESPONSE);

		if (response.contents[0] == 0) {
			std::printf("[STATUS] CNC Connection to <%s> successful.\n", name.c_str());
		} else {
			const std::string error_string = response.extract_string(1);
			char exception_string[1024];
			snprintf(exception_string, sizeof exception_string, "CNC Connection to <%s> failed.\n%s\n", name.c_str(), error_string.c_str());
			throw std::runtime_error(exception_string);
		}
	} catch (...) {
		impl_->client.reset();
		throw;
	}
}

bool remote_control::get_init_state()
{
	if (!impl_->client) {
		throw std::runtime_error("Control not opened.");
	}

	wrap::message message(wrap::message_type::CTRL_GET_INIT);
	wrap::message response = impl_->client->send_message(message, 1000);

	check_server_error(*(impl_->client), response);
	check_correct_response_type(*(impl_->client), response, wrap::message_type::CTRL_GET_INIT_RESPONSE);

	if (response.contents[0] == 0) {
		const bool state = (response.extract_bit8(1) == 1);
		return state;
	}

	const std::string error_string = response.extract_string(1);
	char exception_string[1024];
	snprintf(exception_string, sizeof exception_string, "CNC Getting state for <%s> failed.\n%s\n", impl_->name.c_str(), error_string.c_str());
	throw std::runtime_error(exception_string);
}

init_status remote_control::load_firmware_blocked(std::string const &config_name)
{
	if (!impl_->client) {
		throw std::runtime_error("Control not opened.");
	}

	wrap::message message(wrap::message_type::CTRL_LOAD_FIRMWARE_BLOCKED);
	message.append(config_name);
	wrap::message response = impl_->client->send_message(message, 20000);

	check_server_error(*(impl_->client), response);
	check_correct_response_type(*(impl_->client), response, wrap::message_type::CTRL_LOAD_FIRMWARE_BLOCKED_RESPONSE);

	if (response.contents[0] == 0) {
		return static_cast<init_status>(response.extract_bit8(1));
	}

	const std::string error_string = response.extract_string(1);
	const std::uint32_t win32_error = response.extract_bit32(static_cast<std::uint16_t>(1 + 2 + error_string.size()));

	throw error(error_string, win32_error);
}

void remote_control::send_file_blocked(std::string const &name, std::string const &header,
                                       transfer_block_type type)
{
	if (!impl_->client) {
		throw std::runtime_error("Control not opened.");
	}

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

	wrap::transfer_status_type status = static_cast<wrap::transfer_status_type>(response.extract_bit8(1));
	const std::string error_string = response.extract_string(2);

	if (status == wrap::transfer_status_type::FAIL) {
		const std::uint32_t win32_error = response.extract_bit32(static_cast<std::uint16_t>(2 + 2 + error_string.size()));
		throw transfer_exception_error(error_string, status, win32_error);
	} else {
		throw transfer_exception(error_string, status);
	}
}

void remote_control::send_message(transfer_message const &message)
{
	if (!impl_->client) {
		throw std::runtime_error("Control not opened.");
	}

	wrap::message com_message(wrap::message_type::CTRL_SEND_MESSAGE);

	// HACK: internal message handling here
	com_message.append(static_cast<std::uint16_t>(6 + message.data.size()));

	com_message.append(message.controlblock0);
	com_message.append(message.controlblock1);
	com_message.append(message.controlblock2);
	com_message.append(message.current_block_number);
	com_message.append(message.sender);
	com_message.append(message.handle);

	for (auto const &element: message.data) {
		com_message.append(element);
	}

	wrap::message response = impl_->client->send_message(com_message, 5000);

	check_server_error(*(impl_->client), response);
	check_correct_response_type(*(impl_->client), response, wrap::message_type::CTRL_SEND_MESSAGE_RESPONSE);

	if (response.contents[0] == 0) {
		return;
	}

	const std::string error_string = response.extract_string(1);
	char exception_string[1024];
	snprintf(exception_string, sizeof exception_string, "CNC Sending message to <%s> failed.\n%s\n", impl_->name.c_str(), error_string.c_str());
	throw std::runtime_error(exception_string);
}

void remote_control::read_param_array(std::map<std::uint16_t, double> &parameters)
{
	if (!impl_->client) {
		throw std::runtime_error("Control not opened.");
	}

	wrap::message message(wrap::message_type::CTRL_READ_PARAM_ARRAY);

	message.append(static_cast<std::uint16_t>(parameters.size()));

	for (auto const &parameter: parameters) {
		message.append(parameter.first);
	}

	wrap::message response = impl_->client->send_message(message, 5000);

	check_server_error(*(impl_->client), response);
	check_correct_response_type(*(impl_->client), response, wrap::message_type::CTRL_READ_PARAM_ARRAY_RESPONSE);

	if (response.contents[0] != 0) {
		const std::string error_string = response.extract_string(1);
		const std::uint32_t win32_error = response.extract_bit32(static_cast<std::uint16_t>(1 + 2 + error_string.size()));
		throw error(error_string, win32_error);
	}

	std::uint16_t position = 1;

	for (auto &parameter: parameters) {
		const std::string double_string = response.extract_string(position);
		parameter.second = std::stod(double_string);
		position += static_cast<std::uint16_t>(2 + double_string.size());
	}
}

void remote_control::on_message(callback_type_type type, void *parameter)
{
	assert(impl_->client);
}

}

#ifdef WIN32
namespace
{

wrap::transfer_message eckelmann_to_cpp_message(ULONG parameter)
{
	MSG_TR *em_message = reinterpret_cast<MSG_TR *>(parameter);
	wrap::transfer_message msg = wrap::transfer_message();

	msg.controlblock0 = em_message->sb0_uc;
	msg.controlblock1 = em_message->sb1_uc;
	msg.controlblock2 = em_message->sb2_uc;
	msg.current_block_number = em_message->index_uc;
	msg.handle = em_message->handle_uc;
	msg.sender = em_message->modul_uc;

#define CB(x, y) ((static_cast<std::uint16_t>(x) << 8) | y)

	switch (CB(msg.controlblock0, msg.controlblock1)) {
		case CB(SB0_POLL_KUC, SB1_POLL_KUC): {
			return msg;
		}

		case CB(SB0_AUFTRAG_KUC, SB1_PROGRAMM_ENDE_KUC): {
			msg.data.resize(2);
			std::memcpy(msg.data.data(), &em_message->n.progende_r.quittung_s, 2);
			return msg;
		}

		case CB(SB0_EXCEPTION_KUC, SB1_FEHLERNUMMER_KUC): {
			msg.data.resize(164);

			msg.data[0] = em_message->n.err_meld_r.task_uc;
			msg.data[1] = em_message->n.err_meld_r.klasse_uc;
			std::memcpy(msg.data.data() + 2, &em_message->n.err_meld_r.nummer_s, 2);
			std::memcpy(msg.data.data() + 4, &em_message->n.err_meld_r.info_format_ac[0], 80);
			std::memcpy(msg.data.data() + 84, &em_message->n.err_meld_r.info_data_ac[0], 80);

			return msg;
		}

		case CB(SB0_AUFTRAG_KUC, SB1_PROGNAME_KUC): {
			char string_size = em_message->len_us - 4;
			msg.data.resize(4 + string_size);

			std::uint32_t num;
			std::memcpy(msg.data.data(), &em_message->n.val_auc[0], 4);
			std::memcpy(msg.data.data() + 4, &em_message->n.val_ac[4], string_size);

			return msg;
		}
	}

#undef CB

	char exception_string[1024];
	snprintf(exception_string, sizeof exception_string, "Unable to handle message [cb0: %d, cb1: %d].", msg.controlblock0, msg.controlblock1);
	throw std::runtime_error(exception_string);
}

void do_callback(ULONG type, ULONG parameter, wrap::control *ctrl)
{
	const wrap::callback_type_type cpp_type = conversion_callback_type_type(type);

	switch (type) {
		case VK_MMI_NCMSG_RECEIVED:
		case VK_MMI_ERROR_MSG: {
			wrap::transfer_message msg = eckelmann_to_cpp_message(parameter);
			ctrl->on_message(cpp_type, &msg);
			break;
		}

		default:
			ctrl->on_message(cpp_type, &parameter);
	}
}

void WINAPI callback(ULONG type, ULONG parameter, LPVOID context)
{
	wrap::control *ctrl = NULL;

	for (auto it = g_controls.begin(); it != g_controls.end(); ++it) {
		if (*it == context) {
			ctrl = *it;
			break;
		}
	}

	if (ctrl == NULL) {
		assert(false);
	}

	try {
		do_callback(type, parameter, ctrl);
	} catch (std::exception const &error) {
		ctrl->on_message(wrap::callback_type_type::MMI_UNIMPLEMENTED, const_cast<char *>(error.what()));
	}
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

LONG block_type_conversion(wrap::transfer_block_type type)
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

void eckelmann_cpp_to_msg(wrap::transfer_message const &message, MSG_TR *em_message)
{
	throw;
}

}

#endif

namespace
{

void check_server_error(wrap::client &client, wrap::message const &response)
{
	if (response.type == wrap::message_type::SERVER_ERROR) {
		const std::string error_string = response.extract_string(0);
		char exception_string[1024];
		snprintf(exception_string, sizeof exception_string, "A server error occured while handling client.\n%s", error_string.c_str());
		throw std::runtime_error(exception_string);
	}
}

void check_correct_response_type(wrap::client &client, wrap::message const &response, wrap::message_type type)
{
	if (response.type != type) {
		char exception_string[1024];
		snprintf(exception_string, sizeof exception_string, "Unexpected response type while handling client.\n");
		throw std::runtime_error(exception_string);
	}
}

}
