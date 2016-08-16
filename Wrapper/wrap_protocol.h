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
	message(message_type type);

	void to_bytes(std::vector<std::uint8_t> &bytes) const;
	static std::shared_ptr<message> from_bytes(std::vector<uint8_t> &bytes);

	const std::uint8_t version;
	const message_type type;
	std::uint16_t size;
	std::vector<std::uint8_t> contents;
};

}

#endif
