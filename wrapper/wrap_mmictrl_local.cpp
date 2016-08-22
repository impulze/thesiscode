#include "wrap_server.h"
#include "wrap_protocol.h"
#include "wrap_misc.h"
#include "wrap_mmictrl_local.h"

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

namespace
{

MAKE_FUN_PTR_AND_VAR(ncrOpenControl, HANDLE, LPCSTR, MSGCALLBACK, LPVOID)
MAKE_FUN_PTR_AND_VAR(ncrOpenDefaultControl, HANDLE, MSGCALLBACK, LPVOID)
MAKE_FUN_PTR_AND_VAR(ncrGetInitState, LONG, HANDLE)
MAKE_FUN_PTR_AND_VAR(ncrCloseControl, VOID, HANDLE)
MAKE_FUN_PTR_AND_VAR(ncrLoadFirmwareBlocked, LONG, HANDLE, LPCSTR)
MAKE_FUN_PTR_AND_VAR(ncrSendFileBlocked, LONG, HANDLE, LPCSTR, LPCSTR, LONG)
MAKE_FUN_PTR_AND_VAR(ncrReceiveFileBlocked, LONG, HANDLE, LPCSTR, LONG)
MAKE_FUN_PTR_AND_VAR(ncrSendMessage, BOOL, HANDLE, MSG_TR *)
MAKE_FUN_PTR_AND_VAR(ncrReadParamArray, BOOL, HANDLE, WORD *, double *, WORD)
bool g_dll_loaded;
std::map<wrap::mmictrl_local *, std::function<void(ULONG, ULONG, wrap::mmictrl_local *)>> g_controls;

wrap::transfer_exception create_transfer_exception(wrap::transfer_status_type type);
wrap::transfer_exception_error create_transfer_exception_error(wrap::transfer_status_type type);
void throw_transfer_exception(wrap::transfer_status_type type);
void WINAPI callback(ULONG type, ULONG param, LPVOID context);
wrap::transfer_message eckelmann_to_cpp_message(ULONG parameter);
void load_cnc_dlls();
wrap::error create_error();
LONG to_block_type_conversion(wrap::transfer_block_type type);
wrap::callback_type_type conversion_to_callback_type_type(ULONG type);
wrap::transfer_status_type conversion_to_transfer_status_type(LONG result);

wrap::transfer_exception create_transfer_exception(wrap::transfer_status_type type)
{
	switch (type) {
		case wrap::transfer_status_type::BREAK:
			return wrap::transfer_exception("Transfer cancelled by NC or MMI.", type);
		case wrap::transfer_status_type::TIMEOUT:
			return wrap::transfer_exception("Transfer timed out.", type);
		case wrap::transfer_status_type::NOINIT:
			return wrap::transfer_exception("Transfer failed. Firmware is not loaded yet.", type);
		case wrap::transfer_status_type::QFULL:
			return wrap::transfer_exception("Transfer failed. Queue full.", type);
	}

	assert(false);
	throw std::runtime_error("Internal error");
}

wrap::transfer_exception_error create_transfer_exception_error(wrap::transfer_status_type type)
{
	const DWORD win32_error = GetLastError();
	const std::string error_string = wrap::error_string_from_win32_error(win32_error);

	std::ostringstream strm;

	strm << "Transfer failed. ";
	strm << wrap::error_string_from_win32_error(win32_error);

	return wrap::transfer_exception_error(strm.str(), type, win32_error);
}

void throw_transfer_exception(wrap::transfer_status_type type)
{
	switch (type) {
		case wrap::transfer_status_type::OK:
			return;

		case wrap::transfer_status_type::BREAK:
		case wrap::transfer_status_type::TIMEOUT:
		case wrap::transfer_status_type::NOINIT:
		case wrap::transfer_status_type::QFULL:
			throw create_transfer_exception(type);

		case wrap::transfer_status_type::FAIL:
			throw create_transfer_exception_error(wrap::transfer_status_type::FAIL);

		default:
			assert(false);
	}
}

void WINAPI callback(ULONG type, ULONG parameter, LPVOID context)
{
	wrap::mmictrl_local *ctrl = NULL;
	std::function<void(ULONG, ULONG, wrap::mmictrl_local *)> internal_callback;

	for (auto it = g_controls.begin(); it != g_controls.end(); ++it) {
		if (it->first == context) {
			ctrl = it->first;
			internal_callback = it->second;
			break;
		}
	}

	if (ctrl == NULL) {
		// HACK: why does it do this?
		if (type == VK_MMI_CYCLIC_CALL) {
			return;
		}

		std::fprintf(stderr, "The CNC has sent a message although we already sent ncrClose.\n");
		return;
	}

	try {
		internal_callback(type, parameter, ctrl);
	} catch (std::exception const &exception) {
		std::fprintf(stderr, "Calling message callback failed: %s\nClosing connection.\n", exception.what());
		ctrl->close();
	}

}

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
	LOAD_FUN(ncrReceiveFileBlocked, module)
	LOAD_FUN(ncrSendMessage, module)
	LOAD_FUN(ncrReadParamArray, module)
}

wrap::error create_error()
{
	const DWORD win32_error = GetLastError();
	const std::string error_string = wrap::error_string_from_win32_error(win32_error);

	return wrap::error(error_string, win32_error);
}

LONG to_block_type_conversion(wrap::transfer_block_type type)
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
	}

	throw std::runtime_error("Transfer blocktype not implemented.");
}

wrap::callback_type_type conversion_to_callback_type_type(ULONG type)
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
	}

	throw std::runtime_error("Callback type not implemented.");
}

wrap::transfer_status_type conversion_to_transfer_status_type(LONG result)
{
	switch (result) {
		case TRANSFER_OK: return wrap::transfer_status_type::OK;
		case TRANSFER_BREAK: return wrap::transfer_status_type::BREAK;
		case TRANSFER_TIMEOUT: return wrap::transfer_status_type::TIMEOUT;
		case TRANSFER_NOINIT: return wrap::transfer_status_type::NOINIT;
		case TRANSFER_QFULL: return wrap::transfer_status_type::QFULL;
		case TRANSFER_ERROR: return wrap::transfer_status_type::FAIL;
	}

	throw std::runtime_error("Transfer status type not implemented.");
}

}

namespace wrap
{

struct mmictrl_local::impl
{
	void internal_callback(ULONG type, ULONG parameter, mmictrl_local *ctrl);

	HANDLE handle;
	std::string name;
	transfer_message last_error;
};

void mmictrl_local::impl::internal_callback(ULONG type, ULONG parameter, mmictrl_local *ctrl)
{
	std::vector<std::uint8_t> data;
	const wrap::callback_type_type cpp_type = conversion_to_callback_type_type(type);
	data.resize(5);
	data[0] = static_cast<std::uint8_t>(cpp_type);
	auto func = ctrl->get_message_callback();
	bool called = false;
	transfer_message msg;

	auto msg_append = [&msg, &data]() {
		data.push_back(msg.controlblock0);
		data.push_back(msg.controlblock1);
		data.push_back(msg.controlblock2);
		data.push_back(msg.current_block_number);
		data.push_back(msg.sender);
		data.push_back(msg.handle);
		data.insert(data.end(), msg.data.begin(), msg.data.end());
	};

	auto bit32_to_bit8 = [](std::uint32_t num, std::uint8_t *dest) {
		dest[0] = static_cast<std::uint8_t>((num & 0xff000000) >> 24);
		dest[1] = static_cast<std::uint8_t>((num & 0xff0000) >> 16);
		dest[2] = static_cast<std::uint8_t>((num & 0xff00) >> 8);
		dest[3] = static_cast<std::uint8_t>((num & 0xff) >> 0);
	};

	auto call = [&called, &data, &func, &bit32_to_bit8]() {
		bit32_to_bit8(data.size(), data.data() + 1);
		func(data.data());
		called = true;
	};

	if (type == VK_MMI_NCMSG_RECEIVED || type == VK_MMI_ERROR_MSG) {
		try {
			msg = eckelmann_to_cpp_message(parameter);
		} catch (std::exception const &exception) {
			fprintf(stderr, "Unable to convert NCMSG_RECEIVED/ERROR_MSG: %s\n", exception.what());

			if (func) {
				const size_t string_length = std::strlen(exception.what());
				data[0] = static_cast<std::uint8_t>(wrap::callback_type_type::MMI_UNIMPLEMENTED);
				data.push_back(static_cast<std::uint8_t>(cpp_type));
				data.insert(data.end(), exception.what(), exception.what() + string_length);
				call();
			}

			return;
		}
	}

	switch (cpp_type) {
		case callback_type_type::MMI_CYCLIC_CALL:
			if (func) {
				call();
			}

			break;

		case callback_type_type::MMI_NCMSG_RECEIVED:
			if (func) {
				msg_append();
				call();
			}

			break;

		case callback_type_type::MMI_ERROR_MSG: {
			const std::uint8_t task = msg.data[0];
			const std::uint8_t cls = msg.data[1];
			std::uint16_t num;
			std::memcpy(&num, msg.data.data() + 2, 2);
			const char *format = reinterpret_cast<char *>(msg.data.data() + 4);
			const char *str_data = reinterpret_cast<char *>(msg.data.data() + 84);
			char error_msg[1024];
			snprintf(error_msg, sizeof error_msg, format, str_data);
			last_error = msg;
			std::printf("CNC ERROR: [task: %d, class: %d, num: %hd] [%s]\n", task,  cls, num, error_msg);

			if (func) {
				call();
			}

			break;
		}

		default: {
			if (func) {
				data.resize(data.size() + 4);
				bit32_to_bit8(parameter, data.data() + data.size() - 4);
				call();
			}

			break;
		}
	}

	// this will only run on Win32 consolies
	if (!called) {
		std::printf("Got a message from CNC [type: %d]\n", type);
	}
}


mmictrl_local::~mmictrl_local()
{
	if (impl_) {
		close();
	}
}

void mmictrl_local::open(std::string const &name)
{
	if (impl_) {
		throw std::runtime_error("Control already opened.");
	}

	if (!g_dll_loaded) {
		load_cnc_dlls();
		g_dll_loaded = true;
	}

	std::shared_ptr<impl> impl(new impl());
	impl->handle = NULL;
	impl->name = name;

	auto callback = [impl](ULONG type, ULONG parameter, mmictrl_local *ctrl) {
		impl->internal_callback(type, parameter, ctrl);
	};

	g_controls[this] = callback;

	impl->handle = ncrOpenControl_p(name.c_str(), ::callback, this);

	if (impl->handle == INVALID_HANDLE_VALUE) {
		impl->handle = NULL;
		g_controls.erase(g_controls.find(this));
		throw create_error();
	}

	impl_ = impl;
	std::printf("CNC [%s] opened.\n", impl_->name.c_str());
}

void mmictrl_local::close()
{
	if (!impl_) {
		throw std::runtime_error("Control not opened.");
	}

	ncrCloseControl_p(impl_->handle);

	g_controls.at(this);
	g_controls.erase(g_controls.find(this));

	std::printf("CNC [%s] closed.\n", impl_->name.c_str());
	impl_.reset();
}

bool mmictrl_local::get_init_state()
{
	if (!impl_) {
		throw std::runtime_error("Control not opened.");
	}

	const LONG result = ncrGetInitState_p(impl_->handle);

	if (result == -1) {
		throw std::runtime_error("Getting init state failed.");
	}

	return result == 1;
}

init_status mmictrl_local::load_firmware_blocked(std::string const &config_name)
{
	if (!impl_) {
		throw std::runtime_error("Control not opened.");
	}

	const LONG result = ncrLoadFirmwareBlocked_p(impl_->handle, config_name.c_str());

	if (result == 0) {
		return init_status::FIRMWARE_LOADED;
	} else if (result == -1) {
		return init_status::FIRMWARE_ALREADY_LOADED;
	}

	throw create_error();
}

void mmictrl_local::send_file_blocked(std::string const &name, std::string const &header,
                                transfer_block_type type)
{
	if (!impl_) {
		throw std::runtime_error("Control not opened.");
	}

	const LONG cnv_type = to_block_type_conversion(type);
	const LONG result = ncrSendFileBlocked_p(impl_->handle, name.c_str(), header.c_str(), cnv_type);

	throw_transfer_exception(conversion_to_transfer_status_type(result));
}

void mmictrl_local::receive_file_blocked(std::string const &name, transfer_block_type type)
{
	if (!impl_) {
		throw std::runtime_error("Control not opened.");
	}

	const LONG cnv_type = to_block_type_conversion(type);
	const LONG result = ncrReceiveFileBlocked_p(impl_->handle, header.c_str(), cnv_type);

	throw_transfer_exception(conversion_to_transfer_status_type(result));
}

void mmictrl_local::send_message(transfer_message const &message)
{
	if (!impl_) {
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

	/* HACK:
	 * it seems this function returns false on some occasions without setting any error
	 * in GetLastError() but appearently a message is received in the callback while sending
	 * so clear last error and hope there's a meaningful error after failure
	 */

	wrap::transfer_message msg = {};
	impl_->last_error = msg;
	const BOOL result = ncrSendMessage_p(impl_->handle, &em_message);

	if (result == TRUE) {
		return;
	}

	auto error = create_error();

	if (error.win32_error != 0) {
		throw error;
	}

	if (impl_->last_error.controlblock0 != SB0_EXCEPTION_KUC ||
	    impl_->last_error.controlblock1 != SB1_FEHLERNUMMER_KUC) {
		throw error;
	}

	if (impl_->last_error.sender != 0) {
		throw error;
	}

	std::uint32_t fake_code = (1 << 29);
	std::uint8_t task_uc = impl_->last_error.data[0];
	std::uint16_t nummer_s;

	std::memcpy(&nummer_s, impl_->last_error.data.data() + 2, 2);

	fake_code |= task_uc << 16;
	fake_code |= nummer_s;

	const std::string error_string = wrap::error_string_from_win32_error(fake_code);

	throw wrap::error(error_string, fake_code);
}

void mmictrl_local::read_param_array(std::map<std::uint16_t, double> &parameters)
{
	if (!impl_) {
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
		throw create_error();
	}

	std::vector<double>::const_iterator value_it = values.begin();
	std::vector<std::uint16_t>::const_iterator index_it = indices.begin();

	while (value_it != values.end()) {
		parameters[*index_it] = *value_it;
		++index_it;
		++value_it;
	}
}

}
