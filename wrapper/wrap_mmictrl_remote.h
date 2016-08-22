#ifndef WRAP_MMICTRL_REMOTE_H_INCLUDED
#define WRAP_MMICTRL_REMOTE_H_INCLUDED

#include "wrap_mmictrl.h"

#include <memory>

namespace wrap
{

struct mmictrl_remote
	: mmictrl
{
	mmictrl_remote();
	~mmictrl_remote();

	void open(std::string const &name, std::string const &address, std::uint16_t port);
	void close() override;
	bool get_init_state() override;
	init_status load_firmware_blocked(std::string const &config_name) override;
	void send_file_blocked(std::string const &name, std::string const &header,
	                       transfer_block_type type) override;
	void send_message(transfer_message const &message) override;
	void read_param_array(std::map<std::uint16_t, double> &parameters) override;

private:
	struct impl;

	std::shared_ptr<impl> impl_;
};

}

#endif
