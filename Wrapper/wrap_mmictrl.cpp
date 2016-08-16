#include "wrap_mmictrl.h"

#include <cassert>
#include <codecvt>
#include <cstdio>
#include <stdexcept>
#include <locale>
#include <map>
#include <sstream>

#include "windows.h"

#include "com_1st.h"
#define MMI
#include "com_def.h"
#define _export
#include "mmictrl.h"
#include "com_sbx.h"

#define MMIDBG(...) \
do {\
	SYSTEMTIME lt; \
	GetLocalTime(&lt); \
	std::printf("%04d/%02d/%02d %02d:%02d:%02d.%03d ", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds); \
	std::printf(__VA_ARGS__); \
	std::printf("\n"); \
	std::fflush(stdout); \
} while(0);

#define MAKE_FUN_PTR_AND_VAR(name, ret, ...) \
typedef ret (WINAPI * name ## _fun_ptr)(##__VA_ARGS__); \
name ## _fun_ptr name ## _p;

#define LOAD_FUN(name, module) \
name ## _p = reinterpret_cast<name ## _fun_ptr>(GetProcAddress(module, #name));

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
static std::string error_string_from_win32_error(DWORD win32_error);
static void throw_transfer_exception(LONG result);
static LONG block_type_conversion(wrap::transfer_block_type type);
static wrap::callback_type_type conversion_callback_type_type(LONG type);

namespace wrap
{

struct local_control::local_impl
{
	HANDLE handle;
};

}

namespace wrap
{

error::error(std::string const &message, std::uint32_t win32_error)
	: std::runtime_error(message),
	  win32_error(win32_error)
{
}

error error::create_error()
{
	const DWORD win32_error = GetLastError();
	const std::string error_string = error_string_from_win32_error(win32_error);

	return error(error_string, win32_error);
}

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

transfer_exception_error transfer_exception_error::create(transfer_status_type type)
{
	const DWORD win32_error = GetLastError();
	const std::string error_string = error_string_from_win32_error(win32_error);

	std::ostringstream strm;

	strm << "Transfer failed. ";
	strm << error_string;

	return transfer_exception_error(strm.str(), type, win32_error);
}

control::~control()
{
	for (std::map<LPVOID, control *>::iterator it = g_control_mapping.begin(); it != g_control_mapping.end();) {
		if (it->first == this) {
			it = g_control_mapping.erase(it);
		}
		else {
			++it;
		}
	}
}

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
	}
	catch (...) {
		delete impl_;
		throw;
	}
}

local_control::~local_control()
{
	ncrCloseControl_p(impl_->handle);

	delete impl_;
}

bool local_control::get_init_state()
{
	return ncrGetInitState_p(impl_->handle) == 1;
}

void local_control::load_firmware_blocked(std::string const &config_name)
{
	const LONG result = ncrLoadFirmwareBlocked_p(impl_->handle, config_name.c_str());

	if (result == 0) {
		return;
	} else if (result == -1) {
		return;
	}

	throw error::create_error();
}

void local_control::send_file_blocked(std::string const &name, std::string const &header,
                                transfer_block_type type)
{
	const LONG long_type = block_type_conversion(type);
	const LONG result = ncrSendFileBlocked_p(impl_->handle, name.c_str(), header.c_str(), long_type);

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

}

void WINAPI callback(ULONG type, ULONG param, LPVOID context)
{
	for (auto &entry : g_control_mapping) {
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

std::string error_string_from_win32_error(DWORD win32_error)
{
	if (win32_error & (1 << 29)) {
		const std::uint8_t subsystem = static_cast<std::uint8_t>((win32_error >> 16) & 0xFF);
		const std::uint16_t error_number = static_cast<std::uint16_t>(win32_error & 0x7FFF);

		std::ostringstream strm;

		strm << "Error in Subsystem " << std::to_string(subsystem) << ". ";
		strm << "Error code " << std::to_string(error_number) << ".";

		return strm.str();
	}

	LPWSTR buffer = nullptr;
	size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, win32_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&buffer), 0, NULL);

	const std::wstring message(buffer, size);

	//Free the buffer.
	LocalFree(buffer);

	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	return converter.to_bytes(message);
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