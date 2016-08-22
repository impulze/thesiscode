#include "wrap_protocol.h"
#include "wrap_misc.h"

#include <cassert>
#include <cstddef>
#include <cstring>
#include <limits>
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

	message->size += check_size - 6;
	message->contents.insert(message->contents.end(), bytes.begin() + 6, bytes.begin() + check_size);

	bytes.erase(bytes.begin(), bytes.begin() + message->size);

	return message;
}

std::shared_ptr<message> message::from_type(message_type type)
{
	return std::shared_ptr<message>(new message(type));
}

void message::append(std::string const &string)
{
	const std::uint16_t current_contents_size = static_cast<std::uint16_t>(contents.size());
	const std::uint16_t append_size = static_cast<std::uint16_t>(2 + string.size());

	if (size > (std::numeric_limits<std::uint16_t>::max)() - append_size) {
		throw std::runtime_error("String too long for message.");
	}

	// size of name (2 byte) + name
	size += append_size;
	contents.resize(current_contents_size + append_size);

	const std::uint16_t nlength = htons(static_cast<std::uint16_t>(string.size()));
	std::memcpy(contents.data() + current_contents_size, &nlength, 2);
	std::memcpy(contents.data() + current_contents_size + 2, string.c_str(), string.size());
}

void message::append(std::uint8_t byte)
{
	const std::uint16_t current_contents_size = static_cast<std::uint16_t>(contents.size());
	const std::uint16_t append_size = static_cast<std::uint16_t>(1);

	if (size > (std::numeric_limits<std::uint16_t>::max)() - append_size) {
		throw std::runtime_error("Message already full.");
	}

	size += append_size;
	contents.resize(current_contents_size + append_size);
	*(contents.data() + current_contents_size) = byte;
}

void message::append(std::uint16_t number)
{
	const std::uint16_t current_contents_size = static_cast<std::uint16_t>(contents.size());
	const std::uint16_t append_size = static_cast<std::uint16_t>(2);

	if (size > (std::numeric_limits<std::uint16_t>::max)() - append_size) {
		throw std::runtime_error("Message already full.");
	}

	size += append_size;
	contents.resize(current_contents_size + append_size);

	const std::uint16_t nnumber = htons(number);
	std::memcpy(contents.data() + current_contents_size, &nnumber, 2);
}

void message::append(std::uint32_t number)
{
	const std::uint16_t current_contents_size = static_cast<std::uint16_t>(contents.size());
	const std::uint16_t append_size = static_cast<std::uint16_t>(4);

	if (size > (std::numeric_limits<std::uint16_t>::max)() - append_size) {
		throw std::runtime_error("Message already full.");
	}

	size += append_size;
	contents.resize(current_contents_size + append_size);

	const std::uint32_t nnumber = htonl(number);
	std::memcpy(contents.data() + current_contents_size, &nnumber, 4);
}

std::string message::extract_string(std::uint16_t position) const
{
	if ((contents.size() >= 2 && contents.size() - 2 < position) || contents.size() < 2) {
		throw exception_from_sprintf(1024,
			"Message cannot extract string at this position, no position data [%s,pos=%d].",
			type_to_string(), position);
	}

	std::uint16_t nlength;
	std::memcpy(&nlength, contents.data() + position, 2);
	const std::uint16_t length = ntohs(nlength);

	if (length > contents.size() - position - 2) {
		throw exception_from_sprintf(1024,
			"Message cannot extract string at this position, not enough data [%s,pos=%d].",
			type_to_string(), position);
	}

	std::string string(contents.data() + position + 2, contents.data() + position + 2 + length);

	return string;
}

std::uint8_t message::extract_bit8(std::uint16_t position) const
{
	if ((contents.size() >= 1 && contents.size() - 1 < position) || contents.size() < 1) {
		throw exception_from_sprintf(1024,
			"Message cannot extract byte at this position [%s,pos=%d].",
			type_to_string(), position);
	}

	return contents.data()[position];
}

std::uint16_t message::extract_bit16(std::uint16_t position) const
{
	if ((contents.size() >= 2 && contents.size() - 2 < position) || contents.size() < 2) {
		throw exception_from_sprintf(1024,
			"Message cannot extract word at this position [%s,pos=%d].",
			type_to_string(), position);
	}

	std::uint16_t nnumber;
	std::memcpy(&nnumber, contents.data() + position, 2);
	std::uint16_t number = ntohs(nnumber);

	return number;
}

std::uint32_t message::extract_bit32(std::uint16_t position) const
{
	if ((contents.size() >= 4 && contents.size() - 2 < position) || contents.size() < 4) {
		throw exception_from_sprintf(1024,
			"Message cannot extract double word at this position [%s,pos=%d].",
			type_to_string(), position);
	}

	std::uint32_t nnumber;
	std::memcpy(&nnumber, contents.data() + position, 4);
	std::uint32_t number = ntohl(nnumber);

	return number;
}

char const *message::type_to_string() const
{
	return wrap::message_type_to_string(type);
}

char const *message_type_to_string(message_type type)
{
	switch (type) {
		case message_type::OK: return "OK";
		case message_type::SERVER_ERROR: return "SERVER_ERROR";
		case message_type::CTRL_OPEN: return "CTRL_OPEN";
		case message_type::CTRL_OPEN_RESPONSE: return "CTRL_OPEN_RESPONSE";
		case message_type::CTRL_CLOSE: return "CTRL_CLOSE";
		case message_type::CTRL_CLOSE_RESPONSE: return "CTRL_CLOSE_RESPONSE";
		case message_type::CTRL_GET_INIT: return "CTRL_GET_INIT";
		case message_type::CTRL_GET_INIT_RESPONSE: return "CTRL_GET_INIT_RESPONSE";
		case message_type::CTRL_LOAD_FIRMWARE_BLOCKED: return "CTRL_LOAD_FIRMWARE_BLOCKED";
		case message_type::CTRL_LOAD_FIRMWARE_BLOCKED_RESPONSE: return "CTRL_LOAD_FIRMWRAE_BLOCKED_RESPONSE";
		case message_type::CTRL_SEND_FILE_BLOCKED: return "CTRL_SEND_FILE_BLOCKED";
		case message_type::CTRL_SEND_FILE_BLOCKED_RESPONSE: return "CTRL_SEND_FILE_BLOCKED_RESPONSE";
		case message_type::CTRL_RECEIVE_FILE_BLOCKED: return "CTRL_RECEIVE_FILE_BLOCKED";
		case message_type::CTRL_RECEIVE_FILE_BLOCKED_RESPONSE: return "CTRL_RECEIVE_FILE_BLOCKED_RESPONSE";
		case message_type::CTRL_READ_PARAM_ARRAY: return "CTRL_READ_PARAM_ARRAY";
		case message_type::CTRL_READ_PARAM_ARRAY_RESPONSE: return "CTRL_READ_PARAM_ARRAY_RESPONSE";
		case message_type::CTRL_SEND_MESSAGE: return "CTRL_SEND_MESSAGE";
		case message_type::CTRL_SEND_MESSAGE_RESPONSE: return "CTRL_SEND_MESSAGE_RESPONSE";
		case message_type::CTRL_MESSAGE: return "CTRL_MESSAGE";
	}

	assert(false);
	// make visual studio happy
	throw;
}

}

bool valid_message_type(std::uint16_t check_type)
{
	switch (static_cast<wrap::message_type>(check_type)) {
		case wrap::message_type::OK:
			return true;
		case wrap::message_type::SERVER_ERROR:
			return true;
		case wrap::message_type::CTRL_OPEN:
			return true;
		case wrap::message_type::CTRL_OPEN_RESPONSE:
			return true;
		case wrap::message_type::CTRL_CLOSE:
			return true;
		case wrap::message_type::CTRL_CLOSE_RESPONSE:
			return true;
		case wrap::message_type::CTRL_GET_INIT:
			return true;
		case wrap::message_type::CTRL_GET_INIT_RESPONSE:
			return true;
		case wrap::message_type::CTRL_LOAD_FIRMWARE_BLOCKED:
			return true;
		case wrap::message_type::CTRL_LOAD_FIRMWARE_BLOCKED_RESPONSE:
			return true;
		case wrap::message_type::CTRL_SEND_FILE_BLOCKED:
			return true;
		case wrap::message_type::CTRL_SEND_FILE_BLOCKED_RESPONSE:
			return true;
		case wrap::message_type::CTRL_RECEIVE_FILE_BLOCKED:
			return true;
		case wrap::message_type::CTRL_RECEIVE_FILE_BLOCKED_RESPONSE:
			return true;
		case wrap::message_type::CTRL_READ_PARAM_ARRAY:
			return true;
		case wrap::message_type::CTRL_READ_PARAM_ARRAY_RESPONSE:
			return true;
		case wrap::message_type::CTRL_SEND_MESSAGE:
			return true;
		case wrap::message_type::CTRL_SEND_MESSAGE_RESPONSE:
			return true;
		case wrap::message_type::CTRL_MESSAGE:
			return true;
	}

	return false;
}
