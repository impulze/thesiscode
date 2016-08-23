#include "wrap_server_mmictrl.h"
#include "wrap_mmictrl_local.h"

namespace
{

bool g_winsock_loaded;

template <class T>
bool try_and_fail(T const &func, std::shared_ptr<wrap::message> &response);

struct local_client
{
	std::shared_ptr<wrap::server::client> wrap_client;
	std::shared_ptr<wrap::mmictrl_local> ctrl;
};

}

namespace wrap
{

struct server_mmictrl::impl
{
	void send_response_to_client(std::shared_ptr<server::client> const &client,
	                             std::shared_ptr<message> const &response);
	std::vector<std::shared_ptr<local_client>>::iterator find_local_client(std::shared_ptr<server::client> const &wrap_client);
	void callback(std::uint8_t *data, std::shared_ptr<local_client> const &client, server_mmictrl *server);

	std::vector<std::shared_ptr<local_client>> clients;
	server_mmictrl *server;
};

}

namespace
{

template <class T>
bool try_and_fail(T const &func, std::shared_ptr<wrap::message> &response)
{
	try {
		func();
		return true;
	} catch (wrap::transfer_exception_error const &error) {
		response->append(static_cast<std::uint8_t>(3));
		response->append(error.what());
		response->append(static_cast<std::uint8_t>(error.type));
		response->append(error.win32_error);
	} catch (wrap::error const &error) {
		std::fprintf(stderr, "try_and_fail: %s\n", error.what());
		response->append(static_cast<std::uint8_t>(1));
		response->append(error.what());
		response->append(error.win32_error);
	} catch (wrap::transfer_exception const &exception) {
		response->append(static_cast<std::uint8_t>(4));
		response->append(exception.what());
		response->append(static_cast<std::uint8_t>(exception.type));
	} catch (std::exception const &exception) {
		std::fprintf(stderr, "try_and_fail: %s\n", exception.what());
		response->append(static_cast<std::uint8_t>(2));
		response->append(exception.what());
	}

	return false;
}

}

namespace wrap
{

std::vector<std::shared_ptr<local_client>>::iterator server_mmictrl::impl::find_local_client(std::shared_ptr<server::client> const &wrap_client)
{
	for (auto it = clients.begin(); it != clients.end(); ++it) {
		if ((*it)->wrap_client == wrap_client) {
			return it;
		}
	}

	throw std::runtime_error("Underlying client no longer present in local client list.");
}

void server_mmictrl::impl::callback(std::uint8_t *data, std::shared_ptr<local_client> const &client, server_mmictrl *server)
{
	auto msg = wrap::message::from_type(wrap::message_type::CTRL_MESSAGE);
	std::uint32_t size = 0;

	size |= static_cast<std::uint32_t>(data[1]) << 24;
	size |= static_cast<std::uint32_t>(data[2]) << 16;
	size |= static_cast<std::uint32_t>(data[3]) << 8;
	size |= static_cast<std::uint32_t>(data[4]) << 0;

	msg->append(data[0]);
	msg->append(size);

	for (std::uint16_t i = 0; i < size - 5; i++) {
		msg->append(data[i + 5]);
	}

	server->send_message_to_client(client->wrap_client, msg);
}

server_mmictrl::server_mmictrl(std::string const &address, std::uint16_t port)
	: wrap::server(address, port)
{
	impl_.reset(new impl());
}

void server_mmictrl::on_client_new(std::shared_ptr<client> const &client)
{
	std::shared_ptr<local_client> local_client(new local_client());
	local_client->wrap_client = client;
	local_client->ctrl.reset(new mmictrl_local());
	local_client->ctrl->set_message_callback([this, local_client](std::uint8_t *data) {
		impl_->callback(data, local_client, this);
	});

	impl_->clients.push_back(local_client);
}

void server_mmictrl::on_client_remove(std::shared_ptr<client> const &client)
{
	auto it = impl_->find_local_client(client);

	(*it)->ctrl->set_message_callback([](std::uint8_t *){});
	impl_->clients.erase(it);
}

void server_mmictrl::on_client_message(std::shared_ptr<client> const &client,
                                       std::shared_ptr<message> const &message)
{
	auto lclient_it = impl_->find_local_client(client);
	auto lclient = *lclient_it;
	std::shared_ptr<wrap::message> response;

	switch (message->type) {
		case message_type::CTRL_OPEN: {
			response = message::from_type(message_type::CTRL_OPEN_RESPONSE);
			const std::string name = message->extract_string(0);
			auto const func = [&lclient, &name]() {
				lclient->ctrl->open(name);
			};
			const bool result = try_and_fail(func, response);

			if (result) {
				response->append(static_cast<std::uint8_t>(0));
			}

			break;
		}

		case message_type::CTRL_CLOSE: {
			response = message::from_type(message_type::CTRL_CLOSE_RESPONSE);
			auto const func = [this, lclient_it]() {
				(*lclient_it)->ctrl->set_message_callback([](std::uint8_t *){});
				impl_->clients.erase(lclient_it);
			};
			remove_client(client);
			const bool result = try_and_fail(func, response);

			if (result) {
				response->append(static_cast<std::uint8_t>(0));
			}

			break;
		}

		case message_type::CTRL_GET_INIT: {
			response = message::from_type(message_type::CTRL_GET_INIT_RESPONSE);
			bool state;
			auto const func = [&lclient, &state]() {
				lclient->ctrl->get_init_state();
			};
			const bool result = try_and_fail(func, response);

			if (result) {
				response->append(static_cast<std::uint8_t>(0));
				response->append(static_cast<std::uint8_t>(state ? 1 : 0));
			}

			break;
		}

		case message_type::CTRL_LOAD_FIRMWARE_BLOCKED: {
			response = message::from_type(message_type::CTRL_LOAD_FIRMWARE_BLOCKED_RESPONSE);
			const std::string config_name = message->extract_string(0);
			init_status status;
			auto const func = [&lclient, &config_name, &status]() {
				status = lclient->ctrl->load_firmware_blocked(config_name);
			};
			const bool result = try_and_fail(func, response);

			if (result) {
				response->append(static_cast<std::uint8_t>(0));
				response->append(static_cast<std::uint8_t>(status));
			}

			break;
		}

		case message_type::CTRL_SEND_FILE_BLOCKED: {
			response = message::from_type(message_type::CTRL_SEND_FILE_BLOCKED_RESPONSE);
			const std::string name = message->extract_string(0);
			const std::string header = message->extract_string(static_cast<std::uint16_t>(2 + name.size()));
			const transfer_block_type type = static_cast<transfer_block_type>(message->extract_bit8(static_cast<std::uint16_t>(2 + name.size() + 2 + header.size())));
			auto const func = [&lclient, &name, &header, type]() {
				lclient->ctrl->send_file_blocked(name, header, type);
			};
			const bool result = try_and_fail(func, response);

			if (result) {
				response->append(static_cast<std::uint8_t>(0));
			}

			break;
		}

		case message_type::CTRL_RECEIVE_FILE_BLOCKED: {
			response = message::from_type(message_type::CTRL_RECEIVE_FILE_BLOCKED_RESPONSE);
			const std::string name = message->extract_string(0);
			const transfer_block_type type = static_cast<transfer_block_type>(message->extract_bit8(static_cast<std::uint16_t>(2 + name.size())));
			auto const func = [&lclient, &name, type]() {
				lclient->ctrl->receive_file_blocked(name, type);
			};
			const bool result = try_and_fail(func, response);

			if (result) {
				response->append(static_cast<std::uint8_t>(0));
			}

			break;
		}

		case message_type::CTRL_READ_PARAM_ARRAY: {
			response = message::from_type(message_type::CTRL_READ_PARAM_ARRAY_RESPONSE);
			const std::uint16_t size = message->extract_bit16(0);
			std::uint16_t position = 2;
			std::map<std::uint16_t, double> parameters;

			for (std::uint16_t i = 0; i < size; i++) {
				const std::uint16_t index = message->extract_bit16(position);
				parameters[index] = 0;
				position += 2;
			}

			auto const func = [&lclient, &parameters]() {
				lclient->ctrl->read_param_array(parameters);
			};
			const bool result = try_and_fail(func, response);

			if (result) {
				response->append(static_cast<std::uint8_t>(0));

				for (auto const &parameter : parameters) {
					const std::string double_string = std::to_string(parameter.second);
					response->append(double_string);
				}
			}

			break;
		}

		case message_type::CTRL_SEND_MESSAGE: {
			response = message::from_type(message_type::CTRL_SEND_MESSAGE_RESPONSE);
			transfer_message msg;
			{
				const std::uint16_t size = message->extract_bit16(0);
				msg.controlblock0 = message->extract_bit8(2);
				msg.controlblock1 = message->extract_bit8(3);
				msg.controlblock2 = message->extract_bit8(4);
				msg.current_block_number = message->extract_bit8(5);
				msg.sender = message->extract_bit8(6);
				msg.handle = message->extract_bit8(7);

				std::uint16_t position = 8;

				for (std::uint16_t i = 0; i < size - 6; i++) {
					msg.data.push_back(message->extract_bit8(position++));
				}
			}

			auto const func = [&lclient, &msg]() {
				lclient->ctrl->send_message(msg);
			};
			const bool result = try_and_fail(func, response);

			if (result) {
				response->append(static_cast<std::uint8_t>(0));
			}

			break;
		}
	}

	send_message_to_client(client, response);
}

}