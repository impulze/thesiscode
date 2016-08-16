#ifndef WRAP_PROTOCOL_H_INCLUDED
#define WRAP_PROTOCOL_H_INCLUDED

#include <cstdint>
#include <vector>

namespace wrap
{

enum class message_special : std::uint8_t
{
	MAGIC = 0xE
};

/*
 * Message types (on base of ...):
 * ===============================
 */
enum class message_type : std::uint16_t
{
	CTRL_OPEN,
	CTRL_OPEN_RESPONSE,
};

/*
 * Message structure:
 * ==================
 * VERSION 1 byte
 * TYPE 2 bytes
 * SIZE (in bytes) 1 byte
 * CONTENTS SIZE bytes
 */
struct message
{
	std::uint8_t version;
	message_type type;
	std::uint8_t size;
	std::vector<std::uint8_t> contents;
};

message create_message(message_type type, std::vector<std::uint8_t> const &contents);
std::vector<std::uint8_t> message_to_bytes(message const &message);
std::vector<message> extract_messages_from_buffer(std::vector<std::uint8_t> bytes);

}

#endif
