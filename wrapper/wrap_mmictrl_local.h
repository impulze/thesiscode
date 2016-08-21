#ifndef WRAP_MMICTRL_LOCAL_H_INCLUDED
#define WRAP_MMICTRL_LOCAL_H_INCLUDED

#include "wrap_mmictrl.h"

#include <memory>
#include <string>

#include <windows.h>

namespace wrap
{

struct win32_or_dll_error
	: std::runtime_error
{
	win32_or_dll_error(std::string const &error_string, DWORD win32_error);

	const DWORD win32_error;
};

struct mmictrl_local
	: mmictrl
{
	~mmictrl_local();

	static void initialize();

	void open(std::string const &name);
	bool get_init_state() override;
	init_status load_firmware_blocked(std::string const &config_name) override;
	void send_file_blocked(std::string const &name, std::string const &header,
	                       transfer_block_type type) override;
	void send_message(transfer_message const &message) override;
	void read_param_array(std::map<std::uint16_t, double> &parameters) override;

	void on_message(callback_type_type type, void *parameter) override;

private:
	struct impl;

	std::shared_ptr<impl> impl_;
};

}
#endif