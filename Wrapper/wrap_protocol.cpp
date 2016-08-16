#include "wrap_protocol.h"

#include <cstddef>
#include <stdexcept>

static bool valid_message_type(std::uint16_t check_type);
static std::uint8_t size_for_message_type(wrap::message_type type);

namespace wrap
{

message create_message(message_type type, std::vector<std::uint8_t> const &contents)
{
	if (contents.size() != size_for_message_type(type)) {
		throw std::runtime_error("Invalid size for contents for message.");
	}

	message new_message;
	new_message.version = 1;
	new_message.type = type;
	new_message.size = static_cast<std::uint8_t>(contents.size());
	new_message.contents = contents;

	return new_message;
}

std::vector<message> extract_messages_from_buffer(std::vector<std::uint8_t> bytes)
{
	std::vector<message> messages;

	for (size_t i = 4; i < bytes.size(); i++) {
		const std::uint8_t b0 = bytes[i - 4];
		const std::uint8_t b1 = bytes[i - 3];
		const std::uint8_t b2 = bytes[i - 2];
		const std::uint8_t b3 = bytes[i - 1];
		const std::uint8_t b4 = bytes[i - 0];

		if (b0 != static_cast<std::uint8_t>(message_special::MAGIC)) {
			continue;
		}

		// message version
		if (b1 != 1) {
			continue;
		}

		const std::uint16_t check_type = (static_cast<const std::uint16_t>(b2) << 8) | b3;

		if (!valid_message_type(check_type)) {
			continue;
		}

		const message_type type = static_cast<message_type>(check_type);

		if (size_for_message_type(type) == b4) {
			// check remaining size
			const size_t remaining_size = bytes.size() - (i + 1);

			if (b4 > remaining_size) {
				return messages;
			}
		} else {
			continue;
		}

		const size_t contents_begin = i + 1;

		message new_message;

		new_message.version = b1;
		new_message.type = type;
		new_message.size = b4;
		new_message.contents.assign(bytes.begin() + contents_begin, bytes.begin() + contents_begin + b4);

		messages.push_back(new_message);

		const size_t bytes_size = 5 + new_message.contents.size();

		bytes.erase(bytes.begin() + (i - 4), bytes.begin() + (i - 4) + bytes_size);

		i = 4 - 1;
	}

	return messages;
}

std::vector<std::uint8_t> message_to_bytes(message const &message)
{
	std::vector<std::uint8_t> bytes;

	bytes.push_back(static_cast<std::uint8_t>(message_special::MAGIC));
	bytes.push_back(message.version);
	bytes.push_back((static_cast<std::uint16_t>(message.type) >> 8) & 0xFF);
	bytes.push_back(static_cast<std::uint16_t>(message.type) & 0xFF);
	bytes.push_back(size_for_message_type(message.type));
	bytes.insert(bytes.end(), message.contents.begin(), message.contents.end());

	return bytes;
}

}

bool valid_message_type(std::uint16_t check_type)
{
	switch (static_cast<wrap::message_type>(check_type)) {
		case wrap::message_type::CTRL_OPEN:
			return true;
		case wrap::message_type::CTRL_OPEN_RESPONSE:
			return true;
	}

	return false;
}

std::uint8_t size_for_message_type(wrap::message_type type)
{
	switch (type) {
		case wrap::message_type::CTRL_OPEN:
			return 13;
		case wrap::message_type::CTRL_OPEN_RESPONSE:
			return 14;
	}

	throw std::runtime_error("Unsupported message type.");
}
