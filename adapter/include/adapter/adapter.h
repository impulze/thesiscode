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
	//std::chrono::milliseconds cache_duration;
};

typedef xml_node_type::xml_node_map_type xml_node_map_type;

struct xml_node_fetch_info_type
{
	bool fetched;
	bool cache_valid;
	std::vector<std::uint8_t> bytes;
};

typedef std::function<void(xml_node_type const &node, xml_node_fetch_info_type const &fetch_info)> xml_node_fetch_callback_type;

struct adapter
{
	adapter(xml_node_map_type const &nodes);
	virtual ~adapter();

	void iterate_nodes(std::function<bool(xml_node_type const &node)> const &callback) const;
	virtual void watch_node(xml_node_type const &node, xml_node_fetch_callback_type const &callback) = 0;
	virtual void unwatch_node(xml_node_type const &node) = 0;
	virtual void run() = 0;

protected:
	xml_node_map_type const nodes_;
};

}

#include "adapter_inl.h"

#endif
