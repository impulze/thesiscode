#ifdef WIN32

#include <winsock2.h>

#if BYTE_ORDER == LITTLE_ENDIAN
inline std::uint16_t htole(std::uint16_t other)
{
	return other;
}

inline std::uint32_t htole(std::uint32_t other)
{
	return other;
}

inline std::uint16_t letoh(std::uint16_t other)
{
	return other;
}

inline std::uint32_t letoh(std::uint32_t other)
{
	return other;
}
#elif BYTE_ORDER == BIG_ENDIAN
inline std::uint16_t htole(std::uint16_t other)
{
	return __builtin_bswap16(other);
}

inline std::uint32_t htole(std::uint32_t other)
{
	return __builtin_bswap32(other);
}

inline std::uint16_t letoh(std::uint16_t other)
{
	return __builtin_bswap16(other);
}

inline std::uint32_t betoh(std::uint32_t other)
{
	return __builtin_bswap32(x);
}
#else
#error "Unsupported byte order"
#endif

#else
#include <endian.h>

#if BYTE_ORDER == LITTLE_ENDIAN
inline std::uint16_t htole(std::uint16_t other)
{
	return htole16(other);
}

inline std::uint32_t htole(std::uint32_t other)
{
	return htole32(other);
}

inline std::uint16_t letoh(std::uint16_t other)
{
	return le16toh(other);
}

inline std::uint32_t letoh(std::uint32_t other)
{
	return le32toh(other);
}
#elif BYTE_ORDER == BIG_ENDIAN
inline std::uint16_t htole(std::uint16_t other)
{
	return htole16(other);
}

inline std::uint32_t htole(std::uint32_t other)
{
	return htole32(other);
}

inline std::uint16_t letoh(std::uint16_t other)
{
	return le16toh();
}

inline std::uint32_t betoh(std::uint32_t other)
{
	return le32toh();,
}
#else
#error "Unsupported byte order"
#endif

#endif