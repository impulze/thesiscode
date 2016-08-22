#include "wrap_mmictrl_remote.h"
#include "wrap_client.h"
#include "wrap_protocol.h"
#include "wrap_misc.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#ifdef WIN32
#define snprintf sprintf_s
#else
#define snprintf std::snprintf
#endif

namespace
{

void check_server_error(std::shared_ptr<wrap::message> const &response);
void check_correct_response_type(std::shared_ptr<wrap::message> const &response, wrap::message_type type);
wrap::callback_type_type conversion_to_callback_type(std::uint8_t type);

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
	wrap::client client;
	std::thread client_thread;
	std::mutex mutex;
	std::condition_variable message_received_condition;
	std::shared_ptr<message> last_message_received;
	mmictrl_remote *remote;
	std::exception_ptr exception_ptr;
	bool run_client;
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
		         "A server error occured at remote CNC.\n"
		         "%s\n", error_string.c_str());
		throw std::runtime_error(exception_string);
	}
}

void check_correct_response_type(std::shared_ptr<wrap::message> const &response, wrap::message_type type)
{
	if (response->type != type) {
		char exception_string[1024];
		snprintf(exception_string, sizeof exception_string,
		         "Server sent unexpected response [%s], expected [%s].\n",
		         wrap::message_type_to_string(response->type),
		         wrap::message_type_to_string(type));
		throw std::runtime_error(exception_string);
	}
}

wrap::callback_type_type conversion_to_callback_type(std::uint8_t type)
{
#define CONVHELP(x) case wrap::callback_type_type::x: return wrap::callback_type_type::x;
	switch (static_cast<wrap::callback_type_type>(type)) {
		CONVHELP(MMI_DOWNLOAD_STATE)
		CONVHELP(MMI_DOWNLOAD_PART)
		CONVHELP(MMI_DOWNLOAD_COMPLETE)
		CONVHELP(MMI_DOWNLOAD_ERROR)
		CONVHELP(MMI_TRANSFER_STATE)
		CONVHELP(MMI_TRANSFER_OK)
		CONVHELP(MMI_TRANSFER_ERROR)
		CONVHELP(MMI_TRANSFER_BREAK)
		CONVHELP(MMI_NCMSG_SENT)
		CONVHELP(MMI_NCMSG_NOT_SENT)
		CONVHELP(MMI_NCMSG_RECEIVED)
		CONVHELP(MMI_ERROR_MSG)
		CONVHELP(MMI_CYCLIC_CALL)
		CONVHELP(MMI_DEFAPP_STATE)
		CONVHELP(MMI_CAN_TRANSFER_STATE)
		CONVHELP(MMI_CAN_TRANSFER_COMPLETE)
		CONVHELP(MMI_UNIMPLEMENTED)
	}

	throw std::runtime_error("Conversion not possible.");
}

}

namespace wrap
{

std::shared_ptr<message>
mmictrl_remote::impl::send_message_and_wait(std::shared_ptr<message> const &message, int timeout)
{
if (message->type == wrap::message_type::CTRL_CLOSE) printf("closing impl\n");
printf("interrupting\n");
	client.interrupt();
printf("wakeup condition was true\n");

	client.send_message(message);

	auto stop = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
	std::shared_ptr<wrap::message> response;

	while (true) {
		std::unique_lock<std::mutex> lock(mutex);
		auto status = message_received_condition.wait_until(lock, stop);

printf("message received condition was true\n");
		if (status == std::cv_status::timeout) {
printf("timeout: %d\n", timeout);
			return std::shared_ptr<wrap::message>();
		}

		response = last_message_received;
		last_message_received.reset();

		if (response) {
			check_server_error(response);

			printf("response received: %s\n", response->type_to_string());

			if (response->type == wrap::message_type::CTRL_MESSAGE) {
				wrap::callback_function_type func = remote->get_message_callback();

				if (func) {
					try {
						func(response->contents.data());
					} catch (std::exception const &exception) {
						std::fprintf(stderr, "User callback exception: %s\n", exception.what());
					}
				}

				continue;
			}

			return response;
		}
	}
}

std::shared_ptr<message>
mmictrl_remote::impl::send_message_and_wait(std::shared_ptr<message> const &message, int timeout,
                                            message_type expected_type)
{
	auto response = send_message_and_wait(message, timeout);

	if (!response) {
		throw exception_from_sprintf(1024,
			"Did not receive a message of type [%s] in [%d ms].\n",
			message_type_to_string(expected_type), timeout);
	}

	check_correct_response_type(response, expected_type);

	return response;
}

mmictrl_remote::mmictrl_remote()
{
}

mmictrl_remote::~mmictrl_remote()
{
	if (impl_) {
		close();
	}
}

void mmictrl_remote::open(std::string const &name, std::string const &address, std::uint16_t port)
{
	if (impl_) {
		throw std::runtime_error("[MMICTRL] Control already opened.");
	}

	std::shared_ptr<impl> impl(new mmictrl_remote::impl);

	impl->remote = this;
	impl->name = name;

	try {
		impl->client.connect(address, port);
	} catch (std::exception const &exception) {
		auto newexcp = exception_from_sprintf(1024,
			"[MMICTRL] Opening CNC connection to [%s] failed.\n"
			"[MMICTRL] %s\n", name.c_str(), exception.what());
		throw newexcp;
	}

	std::shared_ptr<message> response;

	impl->run_client = true;
	impl->client_thread = std::thread([impl]() {
		while (impl->run_client) {
			try {
				auto response = impl->client.recv_message();
				impl->last_message_received = response;
				impl->message_received_condition.notify_one();
			} catch (...) {
				impl->exception_ptr = std::current_exception();
			}
		}
	});

	try {
		auto message = message::from_type(message_type::CTRL_OPEN);
		message->append(name);

		response = impl->send_message_and_wait(message, 2000, message_type::CTRL_OPEN_RESPONSE);
	} catch (std::exception const &exception) {
		impl->run_client = false;
		impl->client.interrupt();
		impl->client_thread.join();
		throw exception_from_sprintf(1024,
			"[MMICTRL] Opening CNC connection [%s] failed.\n"
			"[MMICTRL] %s\n", name.c_str(), exception.what());
	}

	if (response->contents[0] == 0) {
		std::printf("[MMICTRL] Opened CNC connection [%s] successfuly.\n", name.c_str());
	} else {
		const std::string error_string = response->extract_string(1);

		impl->run_client = false;
		impl->client.interrupt();
		impl->client_thread.join();

		throw exception_from_sprintf(1024,
			"[MMICTRL] Opening CNC connection [%s] failed.\n"
			"[MMICTRL] Remote error: %s\n", name.c_str(), error_string.c_str());
	}

	impl_ = impl;
}

void mmictrl_remote::close()
{
	if (!impl_) {
		throw std::runtime_error("[MMICTRL] Control not opened.");
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
	impl_.reset();
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

	auto message = message::from_type(message_type::CTRL_SEND_FILE_BLOCKED);
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

void mmictrl_remote::receive_file_blocked(std::string const &name, transfer_block_type type)
{
	if (!impl_) {
		throw std::runtime_error("[MMICTRL] Control not opened.");
	}

	auto message = message::from_type(message_type::CTRL_RECEIVE_FILE_BLOCKED);
	message->append(name);
	message->append(static_cast<std::uint8_t>(type));

	auto response = impl_->send_message_and_wait(message, 20000, message_type::CTRL_RECEIVE_FILE_BLOCKED_RESPONSE);

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

	const std::string error_string = response->extract_string(1);
	const std::uint16_t string_size = static_cast<std::uint16_t>(error_string.size());
	const std::uint32_t win32_error = response->extract_bit32(1 + 2 + string_size);

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

}
