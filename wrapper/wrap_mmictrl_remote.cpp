#include "wrap_mmictrl_remote.h"
#include "wrap_client.h"
#include "wrap_protocol.h"
#include "wrap_misc.h"

#ifdef WIN32
#define snprintf sprintf_s
#else
#define snprintf std::snprintf
#endif

namespace
{

void check_server_error(std::shared_ptr<wrap::message> const &response);
void check_correct_response_type(std::shared_ptr<wrap::message> const &response, wrap::message_type type);

struct mmictrl_client
	: wrap::client
{
	mmictrl_client(std::string const &address, std::uint16_t port);

	// interface is protected, we need to call it from the implementation
	void send_message(std::shared_ptr<wrap::message> const &message);

	void on_message(std::shared_ptr<wrap::message> const &message) override;

	std::shared_ptr<wrap::message> last_message_received;
};

}

namespace wrap
{

struct mmictrl_remote::impl
{
	std::shared_ptr<message> send_message_and_wait(std::shared_ptr<message> const &message,
	                                               int timeout_ms);
	std::shared_ptr<message> send_message_and_wait(std::shared_ptr<message> const &message,
	                                               int timeout, message_type expected_type);

	std::string name;
	std::shared_ptr<mmictrl_client> client;
};

}

namespace
{

void check_server_error(std::shared_ptr<wrap::message> const &response)
{
	if (response->type == wrap::message_type::SERVER_ERROR) {
		const std::string error_string = response->extract_string(0);
		char exception_string[1024];
		snprintf(exception_string, sizeof exception_string,
		         "[MMICTRL] A server error occured at remote CNC.\n"
		         "[MMICTRL] %s\n", error_string.c_str());
		throw std::runtime_error(exception_string);
	}
}

void check_correct_response_type(std::shared_ptr<wrap::message> const &response, wrap::message_type type)
{
	if (response->type != type) {
		char exception_string[1024];
		snprintf(exception_string, sizeof exception_string,
		         "[MMICTRL] Server sent unexpected response [%s], expected [%s].\n",
		         wrap::message_type_to_string(response->type),
		         wrap::message_type_to_string(type));
		throw std::runtime_error(exception_string);
	}
}

mmictrl_client::mmictrl_client(std::string const &address, std::uint16_t port)
	: wrap::client(address, port)
{
}

void mmictrl_client::send_message(std::shared_ptr<wrap::message> const &message)
{
	wrap::client::send_message(message);
}

void mmictrl_client::on_message(std::shared_ptr<wrap::message> const &message)
{
	last_message_received = message;
}

}

namespace wrap
{

std::shared_ptr<message>
mmictrl_remote::impl::send_message_and_wait(std::shared_ptr<message> const &message, int timeout)
{
	client->send_message(message);
	client->last_message_received.reset();
	client->run_one(timeout);

	if (client->last_message_received) {
		check_server_error(client->last_message_received);
	}

	return client->last_message_received;
}

std::shared_ptr<message>
mmictrl_remote::impl::send_message_and_wait(std::shared_ptr<message> const &message, int timeout,
                                            message_type expected_type)
{
	auto response = send_message_and_wait(message, timeout);

	if (!response) {
		throw exception_from_sprintf(1024,
			"Did not receive a message of type [%s] in [%d ms].\n",
			message->type_to_string(), timeout);
	}

	check_correct_response_type(response, expected_type);

	return response;
}

mmictrl_remote::mmictrl_remote()
{
}

mmictrl_remote::~mmictrl_remote()
{
	if (!impl_) {
		// not connected
		return;
	}

	std::shared_ptr<message> response;

	try {
		auto message = message::from_type(message_type::CTRL_CLOSE);
		response = impl_->send_message_and_wait(message, 2000, message_type::CTRL_CLOSE_RESPONSE);
	} catch (std::exception const &exception) {
		std::fprintf(stderr, "[MMICTRL] Closing CNC connection [%s] failed.\n"
		                     "[MMICTRL] %s\n", impl_->name.c_str(), exception.what());
		return;
	}

	if (response->contents[0] == 0) {
		std::printf("[MMICTRL] CNC connection [%s] closed.\n", impl_->name.c_str());
		return;
	}

	const std::string error_string = response->extract_string(1);
	std::fprintf(stderr, "[MMICTRL] Closing CNC connection [%s] failed.\n"
	                     "[MMICTRL] Remote error: %s\n", impl_->name.c_str(), error_string.c_str());
}

void mmictrl_remote::open(std::string const &name, std::string const &address, std::uint16_t port)
{
	std::shared_ptr<impl> impl(new mmictrl_remote::impl());

	impl->name = name;

	try {
		impl->client.reset(new mmictrl_client(address, port));
	} catch (std::exception const &exception) {
		auto newexcp = exception_from_sprintf(1024,
			"[MMICTRL] Opening CNC connection to [%s] failed.\n"
			"[MMICTRL] %s\n", name.c_str(), exception.what());
		throw newexcp;
	}

	std::shared_ptr<message> response;

	try {
		auto message = message::from_type(message_type::CTRL_OPEN);
		message->append(name);

		response = impl->send_message_and_wait(message, 2000, message_type::CTRL_OPEN_RESPONSE);
	} catch (std::exception const &exception) {
		throw exception_from_sprintf(1024,
			"[MMICTRL] Opening CNC connection [%s] failed.\n"
			"[MMICTRL] %s\n", name.c_str(), exception.what());
	}

	if (response->contents[0] == 0) {
		std::printf("[MMICTRL] Opened CNC connection [%s] successfuly.\n", name.c_str());
	} else {
		const std::string error_string = response->extract_string(1);

		throw exception_from_sprintf(1024,
			"[MMICTRL] Opening CNC connection [%s] failed.\n"
			"[MMICTRL] Remote error: %s\n", name.c_str(), error_string.c_str());
	}

	impl_ = impl;
}

bool mmictrl_remote::get_init_state()
{
	if (!impl_) {
		throw std::runtime_error("[MMICTRL] Control not opened.");
	}

	auto message = message::from_type(message_type::CTRL_GET_INIT);
	auto response = impl_->send_message_and_wait(message, 1000, message_type::CTRL_GET_INIT_RESPONSE);

	if (response->contents[0] == 0) {
		const bool state = (response->extract_bit8(1) == 1);
		return state;
	}

	const std::string error_string = response->extract_string(1);
	throw exception_from_sprintf(1024,
		"[MMICTRL] Getting state for CNC [%s] failed.\n"
		"[MMICTRL] %s\n", impl_->name.c_str(), error_string.c_str());
}

init_status mmictrl_remote::load_firmware_blocked(std::string const &config_name)
{
	if (!impl_) {
		throw std::runtime_error("[MMICTRL] Control not opened.");
	}

	auto message = message::from_type(message_type::CTRL_LOAD_FIRMWARE_BLOCKED);
	message->append(config_name);

	auto response = impl_->send_message_and_wait(message, 20000, message_type::CTRL_LOAD_FIRMWARE_BLOCKED_RESPONSE);

	if (response->contents[0] == 0) {
		return static_cast<init_status>(response->extract_bit8(1));
	}

	const std::string error_string = response->extract_string(1);
	const std::uint16_t string_size = static_cast<std::uint16_t>(error_string.size());
	const std::uint32_t win32_error = response->extract_bit32(1 + 2 + string_size);

	char new_error_string[1024];
	snprintf(new_error_string, sizeof new_error_string,
		"[MMICTRL] Remote DLL Win32 or Subsystem error.\n"
		"[MMICTRL] %s\n", error_string.c_str());
	throw error(new_error_string, win32_error);
}

void mmictrl_remote::send_file_blocked(std::string const &name, std::string const &header,
                                       transfer_block_type type)
{
	if (!impl_) {
		throw std::runtime_error("[MMICTRL] Control not opened.");
	}

	auto message = message::from_type(message_type::CTRL_LOAD_FIRMWARE_BLOCKED);
	message->append(name);
	message->append(header);
	message->append(static_cast<std::uint8_t>(type));

	auto response = impl_->send_message_and_wait(message, 20000, message_type::CTRL_SEND_FILE_BLOCKED_RESPONSE);

	if (response->contents[0] == 0) {
		return;
	}

	std::uint16_t position = 0;

	wrap::transfer_status_type status = static_cast<wrap::transfer_status_type>(response->extract_bit8(position++));
	const std::string error_string = response->extract_string(0);
	const std::uint16_t string_size = static_cast<std::uint16_t>(error_string.size());
	char new_error_string[1024];

	if (status == wrap::transfer_status_type::FAIL) {
		const std::uint32_t win32_error = response->extract_bit32(2 + string_size);
		snprintf(new_error_string, sizeof new_error_string,
			"[MMICTRL] Remote DLL Win32 or Subsystem error.\n"
			"[MMICTRL] %s\n", error_string.c_str());
		throw transfer_exception_error(new_error_string, status, win32_error);
	} else {
		snprintf(new_error_string, sizeof new_error_string,
			"[MMICTRL] %s\n", error_string.c_str());
		throw transfer_exception(new_error_string, status);
	}
}

void mmictrl_remote::send_message(transfer_message const &message)
{
	if (!impl_) {
		throw std::runtime_error("[MMICTRL] Control not opened.");
	}

	auto com_message = message::from_type(message_type::CTRL_SEND_MESSAGE);
	// HACK: internal message handling here
	com_message->append(static_cast<std::uint16_t>(6 + message.data.size()));

	com_message->append(message.controlblock0);
	com_message->append(message.controlblock1);
	com_message->append(message.controlblock2);
	com_message->append(message.current_block_number);
	com_message->append(message.sender);
	com_message->append(message.handle);

	for (auto const &element: message.data) {
		com_message->append(element);
	}

	auto response = impl_->send_message_and_wait(com_message, 2000, message_type::CTRL_SEND_MESSAGE_RESPONSE);

	if (response->contents[0] == 0) {
		return;
	}

	const std::string error_string = response->extract_string(0);
	const std::uint16_t string_size = static_cast<std::uint16_t>(error_string.size());
	const std::uint32_t win32_error = response->extract_bit32(2 + string_size);

	char new_error_string[1024];
	snprintf(new_error_string, sizeof new_error_string,
		"[MMICTRL] Remote DLL Win32 or Subsystem error.\n"
		"[MMICTRL] %s\n", error_string.c_str());
	throw error(new_error_string, win32_error);
}

void mmictrl_remote::read_param_array(std::map<std::uint16_t, double> &parameters)
{
	if (!impl_) {
		throw std::runtime_error("[MMICTRL] Control not opened.");
	}

	auto message = message::from_type(message_type::CTRL_READ_PARAM_ARRAY);
	message->append(static_cast<std::uint16_t>(parameters.size()));

	for (auto const &parameter: parameters) {
		message->append(parameter.first);
	}

	auto response = impl_->send_message_and_wait(message, 2000, message_type::CTRL_READ_PARAM_ARRAY_RESPONSE);

	if (response->contents[0] != 0) {
		const std::string error_string = response->extract_string(1);
		const std::uint16_t string_size = static_cast<std::uint16_t>(error_string.size());
		const std::uint32_t win32_error = response->extract_bit32(1 + 2 + string_size);
		char new_error_string[1024];
		snprintf(new_error_string, sizeof new_error_string,
			"[MMICTRL] Remote DLL Win32 or Subsystem error.\n"
			"[MMICTRL] %s\n", error_string.c_str());
		throw error(new_error_string, win32_error);
	}

	std::uint16_t position = 1;

	for (auto &parameter: parameters) {
		const std::string double_string = response->extract_string(position);
		parameter.second = std::stod(double_string);
		position += static_cast<std::uint16_t>(2 + double_string.size());
	}
}

void mmictrl_remote::on_message(callback_type_type type, void *parameter)
{
	std::printf("Received remote MSG: %d\n", static_cast<std::uint8_t>(type));
}

}
