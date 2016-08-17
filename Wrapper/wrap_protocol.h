#ifndef WRAP_PROTOCOL_H_INCLUDED
#define WRAP_PROTOCOL_H_INCLUDED

#include <cstdint>
#include <memory>
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
	OK,
	SERVER_ERROR,
	CTRL_OPEN,
	CTRL_OPEN_RESPONSE,
	CTRL_CLOSE,
	CTRL_CLOSE_RESPONSE,
	CTRL_GET_INIT,
	CTRL_GET_INIT_RESPONSE,
	CTRL_LOAD_FIRMWARE_BLOCKED,
	CTRL_LOAD_FIRMWARE_BLOCKED_RESPONSE,
	CTRL_SEND_FILE_BLOCKED,
	CTRL_SEND_FILE_BLOCKED_RESPONSE,
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
	message(message_type type);

	void to_bytes(std::vector<std::uint8_t> &bytes) const;
	static std::unique_ptr<message> from_bytes(std::vector<uint8_t> &bytes);

	void append(std::string const &string);
	void append(std::uint8_t byte);
	void append(std::uint16_t number);
	void append(std::uint32_t number);

	std::string extract_string(std::uint16_t position) const;
	std::uint8_t extract_bit8(std::uint16_t position) const;
	std::uint16_t extract_bit16(std::uint16_t position) const;
	std::uint32_t extract_bit32(std::uint16_t position) const;

	const std::uint8_t version;
	const message_type type;
	std::uint16_t size;
	std::vector<std::uint8_t> contents;
};

}

#endif
