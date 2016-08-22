#include "wrap_mmictrl.h"

namespace wrap
{

error::error(std::string const &message, std::uint32_t win32_error)
	: std::runtime_error(message),
	  win32_error(win32_error)
{
}

transfer_exception::transfer_exception(std::string const &message, transfer_status_type type)
	: std::runtime_error(message),
	  type(type)
{
}

transfer_exception_error::transfer_exception_error(std::string const &message, transfer_status_type type, std::uint32_t win32_error)
	: std::runtime_error(message),
	  error(message, win32_error),
	  transfer_exception(message, type)
{
}

mmictrl::~mmictrl()
{
}

void mmictrl::reset()
{
	wrap::transfer_message msg = wrap::transfer_message();

	// reset
	msg.controlblock0 = 85;
	msg.controlblock1 = 170;

	send_message(msg);
}

}
