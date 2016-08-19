#ifndef ADAPTER_H_INCLUDED
#define ADAPTER_H_INCLUDED

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <string>

namespace adapter
{

typedef std::map<std::string, std::string> xml_attribute_map_type;

struct xml_node_browse_name
{
	inline operator std::string() const;
	friend bool operator==(xml_node_browse_name const &first, xml_node_browse_name const &second);
	std::string str() const;

	std::string ns;
	std::string name;
};

struct xml_node_browse_path
{
	inline operator std::string() const;
	friend bool operator==(xml_node_browse_path const &first, xml_node_browse_path const &second);

	std::string str() const;
	inline xml_node_browse_name const &last() const;

	std::vector<xml_node_browse_name> elements;
};

struct xml_node_type
{
	typedef std::map<std::string, std::vector<xml_node_type>> xml_node_map_type;

	xml_node_type(std::string const &name, xml_node_browse_path const &browse_path);

	xml_attribute_map_type attributes;
	xml_node_map_type children;
	std::string value;
	std::string name;
	std::string ns;
	xml_node_browse_path browse_path;
};

typedef xml_node_type::xml_node_map_type xml_node_map_type;

struct xml_node_fetch_info_type
{
	bool fetched;
	bool cache_valid;
	std::vector<std::uint8_t> bytes;
};

typedef std::function<void(xml_node_type const &node, xml_node_fetch_info_type const &fetch_info)> xml_node_fetch_callback_type;

enum class value_type
{
	DATA_ITEM,
	ANALOG_ITEM,
	PROPERTY,
	CNC_POSITION_VARIABLE,
};

struct value
{
	value(value_type type);

	const value_type type;
};

struct adapter
{
	adapter(xml_node_map_type const &nodes);
	virtual ~adapter();

	void iterate_nodes(std::function<bool(xml_node_type const &node)> const &callback) const;
	std::shared_ptr<value> get_value_for_browse_path(std::string const &browse_path);
	void watch_node(xml_node_type const &node, xml_node_fetch_callback_type const &callback);
	void unwatch_node(xml_node_type const &node);
	void run();

private:
	struct impl;

	impl *impl_;
};

}

#include "adapter_inl.h"

#endif
