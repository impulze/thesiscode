#include "adapter.h"
#include "wrap_mmictrl.h"

#include <condition_variable>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace
{

#ifdef WIN32
struct my_local_control
	: wrap::remote_control
{
	template <class... T>
	my_local_control(T &&... args);

	void handle_message(wrap::callback_type_type type, unsigned long parameter) override;
};
#endif

struct my_remote_control
	: wrap::remote_control
{
	template <class... T>
	my_remote_control(T &&... args);

	void handle_message(wrap::callback_type_type type, unsigned long parameter) override;
};

void do_iterate_nodes(adapter::xml_node_map_type const &nodes, std::function<bool(adapter::xml_node_type const &node)> callback);
std::string get_attrs(adapter::xml_attribute_map_type const &attributes);
void print_nodes(adapter::xml_node_map_type const &nodes, std::string space = "");

}

namespace adapter
{

struct adapter::impl
{
	impl(xml_node_type const &configuration);

	void setup(xml_node_map_type const &nodes);

	xml_node_type configuration;
	xml_node_map_type nodes;
	std::unique_ptr<wrap::control> ctrl;
	std::map<xml_node_type const *, xml_node_fetch_callback_type> watched_nodes;
	std::map<std::uint16_t, xml_node_type const *> watched_pfields;
	std::mutex mutex;
	std::condition_variable condition;
	bool is_local;
	std::string cnc_name;
	std::string host;
	std::uint16_t port;
};

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

#if 0
std::string xml_node_type::get_browse_path() const
{
	const size_t dotpos = browse_path.rfind(':');

	if (dotpos == std::string::npos) {
		return browse_path;
	}

	return browse_path.substr(dotpos + 1);
}
#endif

value::value(value_type type)
	: type(type)
{
}

adapter::impl::impl(xml_node_type const &configuration)
	: configuration(configuration)
{
}

void adapter::impl::setup(xml_node_map_type const &nodes)
{
	try {
		const xml_node_type position_node = configuration.children.at("position").at(0);

		if (position_node.value == "remote") {
			is_local = false;
		} else if (position_node.value == "local") {
			is_local = true;
		} else {
			throw std::runtime_error("Invalid position string in adapter settings configuration.");
		}
	} catch (std::out_of_range const &) {
		throw std::runtime_error("No position present in adapter settings configuration.");
	}

	try {
		const xml_node_type cnc_name_node = configuration.children.at("name").at(0);

		cnc_name = cnc_name_node.value;
	} catch (std::out_of_range const &) {
		throw std::runtime_error("No name present in adapter settings configuration.");
	}

	try {
		std::vector<xml_node_type> const &objects = configuration.children.at("object");
		this->nodes["object"] = objects;
	} catch (std::out_of_range const &) {
		std::fprintf(stderr, "You have no objects setup in adapter settings.\n");
	}

	if (is_local) {
#ifndef WIN32
		throw std::runtime_error("Local machine controlling only available on Win32.");
#endif
	} else {
		std::string connection_string;

		try {
			const xml_node_type remote_info_node = configuration.children.at("remote_info").at(0);

			connection_string = remote_info_node.value;
		} catch (std::out_of_range const &) {
			throw std::runtime_error("No remote_info present in adapter settings configuration.");
		}


		std::size_t pos = connection_string.rfind(':');

		if (pos == std::string::npos) {
			throw std::runtime_error("No port given in adapter settings remote_info configuration.");
		}

		host = connection_string.substr(0, pos);
		port = static_cast<std::uint16_t>(std::stoi(connection_string.substr(pos + 1)));
	}
}

adapter::adapter(xml_node_map_type const &nodes)
{
	//print_nodes(nodes);

	xml_node_type *configuration;

	try {
		configuration = new xml_node_type(nodes.at("configuration").at(0));
	} catch (std::out_of_range const &) {
		throw std::runtime_error("No configuration present in adapter settings.");
	}

	try {
		impl_ = new adapter::impl(*configuration);
	} catch (...) {
		delete configuration;
		throw;
	}

	delete configuration;

	try {
		impl_->setup(nodes);
	} catch (...) {
		delete impl_;
		throw;
	}
}

adapter::~adapter()
{
	delete impl_;
}

void adapter::iterate_nodes(std::function<bool(xml_node_type const &node)> const &callback) const
{
	do_iterate_nodes(impl_->nodes, callback);
}

std::shared_ptr<value> adapter::get_value_for_browse_path(std::string const &browse_path)
{
	std::shared_ptr<value> value;

	return value;
}

void adapter::watch_node(xml_node_type const &node, xml_node_fetch_callback_type const &callback)
{
	std::unique_lock<std::mutex> lock(impl_->mutex);

	std::printf("watching node: %s\n", node.browse_path.str().c_str());

	std::string pfield;

	try {
		pfield = node.children.at("value").at(0).children.at("pfield").at(0).value;
	} catch (std::out_of_range const &) {
		char exception_string[1024];
		std::snprintf(exception_string, sizeof exception_string, "Node <%s> has no pfield, unable to watch.", node.browse_path.str().c_str());

		if (node.browse_path.last().str() == "cnc:ActGFunctions" ||
		    node.browse_path.last().str() == "cnc:ActMainProgramLine" ||
		    node.browse_path.last().str() == "cnc:ActMainProgramFile") {
			return;
		}

		throw std::runtime_error(exception_string);
	}

	printf("watching\n");
	impl_->watched_nodes[&node] = callback;
	impl_->watched_pfields[static_cast<std::uint16_t>(std::stoi(pfield))] = &node;
	impl_->condition.notify_one();
}

void adapter::unwatch_node(xml_node_type const &node)
{
	std::unique_lock<std::mutex> lock(impl_->mutex);

	impl_->watched_nodes.erase(impl_->watched_nodes.find(&node));
}

void adapter::run()
{
	typedef std::chrono::steady_clock clk;
	typedef std::chrono::time_point<clk> timepoint;
	clk::duration update_interval;

	try {
		const std::string update_interval_string = impl_->configuration.children.at("minimum_communication_interval").at(0).value;
		update_interval = std::chrono::duration_cast<clk::duration>(std::chrono::milliseconds(std::stoi(update_interval_string)));
	} catch (std::out_of_range const &) {
		throw std::runtime_error("Confiuration has no <minimum_communication_interval>.");
	}

#ifdef WIN32
	if (impl_->is_local) {
		impl_->ctrl.reset(new my_local_control(impl_->cnc_name));
	} else {
#endif
		impl_->ctrl.reset(new my_remote_control(impl_->cnc_name, impl_->host, impl_->port));
#ifdef WIN32
	}
#endif

	timepoint last_update_try = clk::now() - update_interval;

	while (true) {
		std::unique_lock<std::mutex> lock(impl_->mutex);

		while (impl_->watched_nodes.empty()) {
			impl_->condition.wait(lock);
		}

		const timepoint now = clk::now();
		const clk::duration diff = now - last_update_try;

		printf("diff from last: %ld\n", std::chrono::duration_cast<std::chrono::milliseconds>(diff).count());

		if (diff < update_interval) {
			std::printf("Not stressing communication. Update is queued.\n");
			std::this_thread::sleep_for(update_interval - diff);
			std::printf("done sleeping: %ld\n", std::chrono::duration_cast<std::chrono::milliseconds>(update_interval - diff).count());
		}

		last_update_try = clk::now();

		std::ostringstream strm;

		for (auto const &entry: impl_->watched_nodes) {
			strm << entry.first->browse_path.str();
			strm << ",";
		}

		std::map<std::uint16_t, double> parameters;

		for (auto const &entry: impl_->watched_pfields) {
			parameters[entry.first] = 0;
		}

		if (parameters.size() > 0) {
			lock.unlock();

			impl_->ctrl->read_param_array(parameters);

			lock.lock();
		}

		printf("read param returned\n");

		for (auto const &entry: parameters) {
			auto pit = impl_->watched_pfields.find(entry.first);

			if (pit == impl_->watched_pfields.end()) {
				std::printf("P-Field <%hu> was removed during CNC communication. Discard value.\n", entry.first);
				continue;
			}

			auto nit = impl_->watched_nodes.find(pit->second);

			if (nit == impl_->watched_nodes.end()) {
				std::printf("Node <%s> was removed during CNC communication. Discard value.\n", pit->second->browse_path.str().c_str());
				continue;
			}

			const std::string double_string = std::to_string(entry.second);

			xml_node_fetch_info_type fetch_info;

			fetch_info.fetched = true;
			fetch_info.bytes.insert(fetch_info.bytes.end(), double_string.begin(), double_string.end());
			nit->second(*(nit->first), fetch_info);
		}

		lock.unlock();
	}
}

}

namespace
{

#ifdef WIN32
template <class... T>
my_local_control::my_remote_control(T &&... args)
	: wrap::local_control(std::forward<T>(args)...)
{
}

void my_local_control::handle_message(wrap::callback_type_type type, unsigned long parameter)
{
	std::printf("new message: %d %lu\n", static_cast<int>(type), parameter);
}

#endif

template <class... T>
my_remote_control::my_remote_control(T &&... args)
	: wrap::remote_control(std::forward<T>(args)...)
{
}

void my_remote_control::handle_message(wrap::callback_type_type type, unsigned long parameter)
{
	std::printf("new message: %d %lu\n", static_cast<int>(type), parameter);
}

void do_iterate_nodes(adapter::xml_node_map_type const &nodes, std::function<bool(adapter::xml_node_type const &node)> callback)
{
	for (auto const &entry: nodes) {
		if (entry.first != "object" && entry.first != "variable") {
			continue;
		}

		for (auto const &node: entry.second) {
			if (!callback(node)) {
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
