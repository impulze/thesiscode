#include "wrap_protocol.h"

#include <cstddef>
#include <cstring>
#include <stdexcept>

#ifdef WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

static bool valid_message_type(std::uint16_t check_type);

namespace wrap
{

message::message(message_type type)
	: version(1),
	  type(type),
	  size(6)
{
}

void message::to_bytes(std::vector<std::uint8_t> &bytes) const
{
	bytes.resize(6);

	bytes[0] = static_cast<std::uint8_t>(message_special::MAGIC);
	bytes[1] = version;

	const std::uint16_t message_type = htons(static_cast<std::uint16_t>(type));
	const std::uint16_t message_size = htons(size);

	std::memcpy(bytes.data() + 2, &message_type, 2);
	std::memcpy(bytes.data() + 4, &message_size, 2);

	bytes.insert(bytes.end(), contents.begin(), contents.end());
}

std::shared_ptr<message> message::from_bytes(std::vector<std::uint8_t> &bytes)
{
	std::shared_ptr<message> message;

	if (bytes.size() < 6) {
		std::printf("Not enough message data yet.\n");
		return message;
	}

	if (bytes[0] != static_cast<std::uint8_t>(message_special::MAGIC)) {
		std::fprintf(stderr, "Invalid message format.\n");
		bytes.erase(bytes.begin(), bytes.begin() + 1);
		return message;
	}

	if (bytes[1] != 1) {
		std::fprintf(stderr, "Invalid message version.\n");
		bytes.erase(bytes.begin(), bytes.begin() + 2);
		return message;
	}

	std::uint16_t check_type;

	std::memcpy(&check_type, bytes.data() + 2, 2);

	check_type = ntohs(check_type);

	if (!valid_message_type(check_type)) {
		std::fprintf(stderr, "Invalid message type.\n");
		bytes.erase(bytes.begin(), bytes.begin() + 4);
		return message;
	}

	std::uint16_t check_size;

	std::memcpy(&check_size, bytes.data() + 4, 2);

	check_size = ntohs(check_size);

	if (check_size < 6) {
		std::fprintf(stderr, "Insufficient message size.\n");
		bytes.erase(bytes.begin(), bytes.begin() + 6);
		return message;
	}

	if (check_size > bytes.size()) {
		std::printf("Not enough message data yet.\n");
		return message;
	}

	message.reset(new wrap::message(static_cast<message_type>(check_type)));

	message->size += static_cast<std::uint16_t>(bytes.size() - 6);
	message->contents.insert(message->contents.end(), bytes.begin() + 6, bytes.end());

	bytes.erase(bytes.begin(), bytes.begin() + message->size);

	return message;
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
