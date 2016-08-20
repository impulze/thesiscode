#include <adapter/adapter.h>

#include <sstream>

namespace
{

void do_iterate_nodes(adapter::xml_node_map_type const &nodes, std::function<bool(adapter::xml_node_type const &node)> callback);
std::string get_attrs(adapter::xml_attribute_map_type const &attributes);
void print_nodes(adapter::xml_node_map_type const &nodes, std::string space = "");

}

namespace adapter
{

bool operator==(xml_node_browse_name const &first, xml_node_browse_name const &second)
{
	return first.ns == second.ns && first.name == second.name;
}

std::string xml_node_browse_name::str() const
{
	std::ostringstream strm;

	// TODO: define prefixes in .xml?
	if (ns == "http://opcfoundation.org/UA/") {
		strm << "ua:";
	} else if (ns == "http://opcfoundation.org/UA/CNC/") {
		strm << "cnc:";
	} else if (ns == "http://cs.hs-rm.de/mierswa_thesis/CNC/") {
		strm << "thesis_cnc:";
	} else if (ns == "http://cs.hs-rm.de/mierswa_thesis/Eckelmann/") {
		strm << "eckelmann_cnc:";
	} else if (ns == "http://cs.hs-rm.de/mierswa_thesis/3AchsenEMCNCLaserSchneid/") {
		strm << "demo:";
	} else {
		strm << "UNKNOWN:";
	}

	strm << name;

	return strm.str();
}

bool operator==(xml_node_browse_path const &first, xml_node_browse_path const &second)
{
	return first.elements == second.elements;
}

std::string xml_node_browse_path::str() const
{
	std::ostringstream strm;

	for (auto const &element: elements) {
		strm << "/";
		strm << static_cast<std::string>(element);
	}

	return strm.str();
}

xml_node_type::xml_node_type(std::string const &name, xml_node_browse_path const &browse_path)
	: name(name),
	  browse_path(browse_path)
{
}

adapter::adapter(xml_node_map_type const &nodes)
	: nodes_(nodes)
{
	//print_nodes(nodes);
}

adapter::~adapter()
{
}

void adapter::iterate_nodes(std::function<bool(xml_node_type const &node)> const &callback) const
{
	do_iterate_nodes(nodes_, callback);
}

}

namespace
{

void do_iterate_nodes(adapter::xml_node_map_type const &nodes, std::function<bool(adapter::xml_node_type const &node)> callback)
{
	for (auto const &entry: nodes) {
		bool call_callback = true;

		if (entry.first != "object" && entry.first != "variable") {
			call_callback = false;
		}

		for (auto const &node: entry.second) {
			if (call_callback && !callback(node)) {
				return;
			}

			do_iterate_nodes(node.children, callback);
		}
	}
}

std::string get_attrs(adapter::xml_attribute_map_type const &attributes)
{
	std::ostringstream strm;

	strm << "[";

	for (adapter::xml_attribute_map_type::const_iterator it = attributes.begin();
	     it != attributes.end();
	     ++it) {
		auto const &attribute = *it;

		strm << attribute.first << "=" << attribute.second;
		strm << ",";
	}

	strm << "]";

	return strm.str();
}

void print_nodes(adapter::xml_node_map_type const &nodes, std::string space)
{
	std::string new_space = space + "  ";

	for (auto const &entry: nodes) {
		for (auto const &node: entry.second) {
			std::printf("%sname: %s\n", space.c_str(), node.name.c_str());
			std::printf("%sattributes: %s\n", space.c_str(), get_attrs(node.attributes).c_str());
			print_nodes(node.children, new_space);
		}
	}
}

}
