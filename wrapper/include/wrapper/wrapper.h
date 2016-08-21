#ifndef WRAPPER_H_INCLUDED
#define WRAPPER_H_INCLUDED

#include <adapter/adapter.h>

namespace wrapper
{

struct wrapper
	: adapter::adapter
{
	wrapper(::adapter::xml_node_map_type const &nodes);
	virtual ~wrapper();

	void watch_node(::adapter::xml_node_type const &node, ::adapter::xml_node_fetch_callback_type const &callback) override;
	void unwatch_node(::adapter::xml_node_type const &node) override;
	void run() override;
	void set_error_callback(::adapter::error_callback_type const &callback) override;

private:
	struct impl;

	impl *impl_;
};

}

#endif
