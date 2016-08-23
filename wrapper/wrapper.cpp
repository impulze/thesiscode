#include <wrapper/wrapper.h>
#include "wrap_mmictrl.h"
#ifdef WIN32
#include "wrap_mmictrl_local.h"
#endif
#include "wrap_mmictrl_remote.h"

#include <condition_variable>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <thread>

#ifdef WIN32
#define snprintf sprintf_s
#else
#define snprintf std::snprintf
#endif

// debug
#include <bitset>
#include <iostream>

namespace wrapper
{

typedef std::function<void(::adapter::xml_node_type const &, ::adapter::xml_node_fetch_info_type &)>
	internal_function_callback_type;

struct wrapper::impl
{
	impl(::adapter::xml_node_type const &configuration);

	void setup(::adapter::xml_node_map_type const &nodes);
	void cnc_callback(std::uint8_t *data, wrapper *wrapper);

	::adapter::xml_node_type configuration;
	::adapter::xml_node_map_type nodes;
	std::unique_ptr<wrap::mmictrl> ctrl;
	std::map<::adapter::xml_node_type const *, internal_function_callback_type> watched_nodes;
	std::map<std::uint16_t, std::vector<::adapter::xml_node_type const *>> watched_pfields;
	std::mutex mutex;
	std::condition_variable condition;
	bool is_local;
	std::string cnc_name;
	std::string host;
	std::uint16_t port;
};

}

// implementations

namespace wrapper
{

wrapper::impl::impl(::adapter::xml_node_type const &configuration)
	: configuration(configuration)
{
}

void wrapper::impl::setup(::adapter::xml_node_map_type const &nodes)
{
	try {
		const ::adapter::xml_node_type position_node = configuration.children.at("position").at(0);

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
		const ::adapter::xml_node_type cnc_name_node = configuration.children.at("name").at(0);

		cnc_name = cnc_name_node.value;
	} catch (std::out_of_range const &) {
		throw std::runtime_error("No name present in adapter settings configuration.");
	}

	try {
		std::vector<::adapter::xml_node_type> const &objects = configuration.children.at("object");
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
			const ::adapter::xml_node_type remote_info_node = configuration.children.at("remote_info").at(0);

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

void wrapper::impl::cnc_callback(std::uint8_t *data, wrapper *wrapper)
{
	const wrap::callback_type_type cpp_type = static_cast<wrap::callback_type_type>(data[0]);

	std::uint32_t size = 0;
	size |= static_cast<std::uint32_t>(data[1]) << 24;
	size |= static_cast<std::uint32_t>(data[2]) << 16;
	size |= static_cast<std::uint32_t>(data[3]) << 8;
	size |= static_cast<std::uint32_t>(data[4]) << 0;

	if (cpp_type == wrap::callback_type_type::MMI_NCMSG_RECEIVED) {
		wrap::transfer_message msg;
		msg.controlblock0 = data[5];
		msg.controlblock1 = data[6];
		msg.controlblock2 = data[7];
		msg.current_block_number = data[8];
		msg.sender = data[9];
		msg.handle = data[10];
		msg.data.insert(msg.data.end(), data + 11, data + 11 + (size - 11));

		if (msg.controlblock0 == 3 && msg.controlblock1 == 22) {
			std::uint32_t prognum;
			std::memcpy(&prognum, msg.data.data(), 4);
			const std::string name(msg.data.data() + 4, msg.data.data() + msg.data.size());
			std::printf("Progname response: [%d=%s]\n", prognum, name.c_str());
		} else {
			std::printf("NCMSG: [cb0=%d,cb1=%d]\n", msg.controlblock0, msg.controlblock1);
		}
	} else {
		std::printf("Callback: [type=%d]\n", data[0]);
	}
}

wrapper::wrapper(::adapter::xml_node_map_type const &nodes)
	: ::adapter::adapter(nodes)
{
	::adapter::xml_node_type *configuration;

	try {
		configuration = new ::adapter::xml_node_type(nodes.at("configuration").at(0));
	} catch (std::out_of_range const &) {
		throw std::runtime_error("No configuration present in adapter settings.");
	}

	try {
		impl_ = new wrapper::impl(*configuration);
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

wrapper::~wrapper()
{
	delete impl_;
}

void wrapper::watch_node(::adapter::xml_node_type const &node, ::adapter::xml_node_fetch_callback_type const &callback)
{
	std::unique_lock<std::mutex> lock(impl_->mutex);

	const std::string browse_name = node.browse_path.last().str();

	typedef ::adapter::xml_node_type const &ntype;
	typedef ::adapter::xml_node_fetch_callback_type const &fcbtype;
	typedef ::adapter::xml_node_fetch_info_type &fitype;

	bool pfield_set = false;
	std::string pfield;

	try {
		pfield = node.children.at("value").at(0).children.at("pfield").at(0).value;
		pfield_set = true;
	} catch (std::out_of_range const &) {
	}

	if (pfield_set) {
		if (node.browse_path.last().str() == "cnc:ActProgramStatus") {
			auto status_callback = [callback](ntype node, fitype fetch_info) {
				const std::string double_string(fetch_info.bytes.begin(), fetch_info.bytes.end());
				const double value = std::stod(double_string);
				const std::uint8_t status = static_cast<std::uint8_t>(value);

				fetch_info.bytes.resize(1);
				if (status == 0) { // idle
					fetch_info.bytes[0] = 0; // stopped
				} else if (status == 1) { // run
					fetch_info.bytes[0] = 1; // run
				} else if (status == 2) { // break-abort
					fetch_info.bytes[0] = 4; // canceled
				} else if (status == 3) { // abort-wait-quit
					fetch_info.bytes[0] = 2; // wait
				} else if (status == 4) { // abort
					fetch_info.bytes[0] = 4; // canceled
				} else if (status == 5) { // break-interrupt (unterbrechen)
					fetch_info.bytes[0] = 3; // interrupt
				} else if (status == 6) { // interrupt (unterbrechen)
					fetch_info.bytes[0] = 3; // interrupt
				} else if (status == 7) { // break-next
					fetch_info.bytes[0] = 0; // run
				} else if (status == 8) { // next
					fetch_info.bytes[0] = 0; // run
				} else if (status == 9) { // break-interrupt
					fetch_info.bytes[0] = 3; // interrupt
				} else if (status == 10) { // interrupt
					fetch_info.bytes[0] = 3; // interrupt
				} else if (status == 11) { // break-error
					fetch_info.bytes[0] = 3; // interrupt
				} else if (status == 12) { // error
					fetch_info.bytes[0] = 3; // interrupt
				} else if (status == 13) { // ausgleich
					fetch_info.bytes[0] = 1; // run
				}

				callback(node, fetch_info);
			};
			impl_->watched_nodes[&node] = status_callback;
			impl_->watched_pfields[static_cast<std::uint16_t>(std::stoi(pfield))].push_back(&node);
		} else if (node.browse_path.last().str() == "cnc:ActStatus") {
			auto status_callback = [callback](ntype node, fitype fetch_info) {
				const std::string double_string(fetch_info.bytes.begin(), fetch_info.bytes.end());
				const double value = std::stod(double_string);
				const std::uint8_t status = static_cast<std::uint8_t>(value);

				fetch_info.bytes.resize(1);
				// TODO: reset is not used?
				if (status == 0) { // idle
					fetch_info.bytes[0] = 0; // active
				} else if (status == 1) { // run
					fetch_info.bytes[0] = 0; // active
				} else if (status == 2) { // break-abort
					fetch_info.bytes[0] = 1; // interrupted
				} else if (status == 3) { // abort-wait-quit
					fetch_info.bytes[0] = 0; // active
				} else if (status == 4) { // abort
					fetch_info.bytes[0] = 1; // interrupted
				} else if (status == 5) { // break-interrupt (unterbrechen)
					fetch_info.bytes[0] = 1; // interrupted
				} else if (status == 6) { // interrupt (unterbrechen)
					fetch_info.bytes[0] = 1; // interrupted
				} else if (status == 7) { // break-next
					fetch_info.bytes[0] = 0; // active
				} else if (status == 8) { // next
					fetch_info.bytes[0] = 0; // active
				} else if (status == 9) { // break-interrupt
					fetch_info.bytes[0] = 1; // interrupted
				} else if (status == 10) { // interrupt
					fetch_info.bytes[0] = 1; // interrupted
				} else if (status == 11) { // break-error
					fetch_info.bytes[0] = 1; // interrupted
				} else if (status == 12) { // error
					fetch_info.bytes[0] = 1; // interrupted
				} else if (status == 13) { // ausgleich
					fetch_info.bytes[0] = 0; // run
				}

				callback(node, fetch_info);
			};
			impl_->watched_nodes[&node] = status_callback;
			impl_->watched_pfields[static_cast<std::uint16_t>(std::stoi(pfield))].push_back(&node);
		} else if (node.browse_path.last().str() == "cnc:ActGFunctions") {
			auto gfunc_callback = [callback](ntype node, fitype fetch_info) {
				const std::string double_string(fetch_info.bytes.begin(), fetch_info.bytes.end());
				const std::bitset<10> bits(static_cast<unsigned long long>(std::stod(double_string)));
				std::vector<std::uint32_t> array;

				if (bits[0]) {
					array.push_back(31);
				} else {
					array.push_back(30);
				}

				if (bits[2]) {
					array.push_back(133);
				} else {
					array.push_back(132);
				}

				if (bits[4]) {
					array.push_back(114);
				}

				if (bits[5]) {
					array.push_back(231);
				}

				if (bits[6]) {
					array.push_back(217);
				}

				if (bits[8]) {
					array.push_back(214);
				}

				if (bits[9]) {
					array.push_back(233);
				}

				std::uint32_t array_size = array.size();

				fetch_info.bytes.resize(4 + array.size() * 4);
				std::memcpy(fetch_info.bytes.data(), &array_size, 4);

				size_t current = 4;

				for (auto const &element: array) {
					std::memcpy(fetch_info.bytes.data() + current, &element, 4);
					current += 4;
				}
	
				callback(node, fetch_info);
			};
			impl_->watched_nodes[&node] = gfunc_callback;
			impl_->watched_pfields[static_cast<std::uint16_t>(std::stoi(pfield))].push_back(&node);
		} else if (node.browse_path.last().str() == "cnc:ActOperationMode") {
			auto oper_callback = [callback](ntype node, fitype fetch_info) {
				const std::string double_string(fetch_info.bytes.begin(), fetch_info.bytes.end());
				const double value = std::stod(double_string);
				const std::uint8_t operation_mode = static_cast<std::uint8_t>(value);

				fetch_info.bytes.resize(1);
				if (operation_mode == 0) {
					// kein programm = manual
					fetch_info.bytes[0] = 0;
				} else {
					// alles andere = automatisch (kein MDA)
					fetch_info.bytes[0] = 2;
				}
				callback(node, fetch_info);
			};
			impl_->watched_nodes[&node] = oper_callback;
			impl_->watched_pfields[static_cast<std::uint16_t>(std::stoi(pfield))].push_back(&node);
		} else {
			auto pfield_callback = [callback](ntype node, fitype fetch_info) {
				// append size of double string
				const std::uint32_t size = fetch_info.bytes.size();
				std::uint8_t const *size_ptr = reinterpret_cast<std::uint8_t const *>(&size);
				fetch_info.bytes.insert(fetch_info.bytes.begin(), size_ptr, size_ptr + 4);
				callback(node, fetch_info);
			};
			impl_->watched_nodes[&node] = pfield_callback;
			impl_->watched_pfields[static_cast<std::uint16_t>(std::stoi(pfield))].push_back(&node);
		}
	} else {
		//if (browse_name == "cnc:ActMainProgramFile") return;
		//if (browse_name == "cnc:ActMainProgramLine") return;

		char exception_string[1024];
		snprintf(exception_string, sizeof exception_string, "Node <%s> has no pfield and no custom code. Unable to watch.", node.browse_path.str().c_str());
		throw std::runtime_error(exception_string);
	}

	impl_->condition.notify_one();
}

void wrapper::unwatch_node(::adapter::xml_node_type const &node)
{
	std::unique_lock<std::mutex> lock(impl_->mutex);

	impl_->watched_nodes.erase(impl_->watched_nodes.find(&node));
}

void wrapper::run()
{
#ifndef WIN32
	typedef std::chrono::steady_clock clk;
#else
	typedef std::chrono::high_resolution_clock clk;
#endif

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
		std::unique_ptr<wrap::mmictrl_local> lctrl(new wrap::mmictrl_local());
		lctrl->open(impl_->cnc_name);
		impl_->ctrl = std::move(lctrl);
	} else {
#endif
		std::unique_ptr<wrap::mmictrl_remote> rctrl(new wrap::mmictrl_remote());
		rctrl->open(impl_->cnc_name, impl_->host, impl_->port);
		impl_->ctrl = std::move(rctrl);
#ifdef WIN32
	}
#endif

	impl_->ctrl->set_message_callback([this](std::uint8_t *data) {
		impl_->cnc_callback(data, this);
	});

	timepoint last_update_try = clk::now();
	last_update_try -= update_interval;

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

		for (auto const &entry: parameters) {
std::cout << "PFIELD[" << entry.first << "=" << entry.second << "]\n";
			auto pit = impl_->watched_pfields.find(entry.first);

			if (pit == impl_->watched_pfields.end()) {
				std::printf("P-Field <%hu> was removed during CNC communication. Discard value.\n", entry.first);
				continue;
			}

			for (auto &elem: pit->second) {
				auto nit = impl_->watched_nodes.find(elem);

				if (nit == impl_->watched_nodes.end()) {
					std::printf("Node <%s> was removed during CNC communication. Discard value.\n", elem->browse_path.str().c_str());
					continue;
				}

				const std::string double_string = std::to_string(entry.second);

				::adapter::xml_node_fetch_info_type fetch_info;

				fetch_info.fetched = true;
				fetch_info.bytes.insert(fetch_info.bytes.end(), double_string.begin(), double_string.end());
				nit->second(*(nit->first), fetch_info);
			}
		}

		lock.unlock();
	}
}

void wrapper::set_error_callback(::adapter::error_callback_type const &callback)
{
}

}
