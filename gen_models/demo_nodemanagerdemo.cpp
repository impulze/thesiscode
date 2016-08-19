/******************************************************************************
** demo_nodemanagerdemo.cpp
**
**   **************************** auto-generated ****************************
**     This code was generated by a tool: UaModeler
**     Runtime Version: 1.5.0, using C++ OPC UA SDK 1.5.0 template (version 4)
**
**     This is a template file that was generated for your convenience.
**     This file will not be overwritten when generating code again.
**     ADD YOUR IMPLEMTATION HERE!
**   **************************** auto-generated ****************************
**
** Copyright (c) 2006-2016 Unified Automation GmbH. All rights reserved.
**
** Software License Agreement ("SLA") Version 2.5
**
** Unless explicitly acquired and licensed from Licensor under another
** license, the contents of this file are subject to the Software License
** Agreement ("SLA") Version 2.5, or subsequent versions
** as allowed by the SLA, and You may not copy or use this file in either
** source code or executable form, except in compliance with the terms and
** conditions of the SLA.
**
** All software distributed under the SLA is provided strictly on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
** AND LICENSOR HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT
** LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
** PURPOSE, QUIET ENJOYMENT, OR NON-INFRINGEMENT. See the SLA for specific
** language governing rights and limitations under the SLA.
**
** The complete license agreement can be found here:
** http://unifiedautomation.com/License/SLA/2.5/
**
** Project: C++ OPC Server SDK information model for namespace http://cs.hs-rm.de/mierswa_thesis/3AchsenEMCNCLaserSchneid/
**
** Description: OPC Unified Architecture Software Development Kit.
**
** Created: 19.08.2016
**
******************************************************************************/

#include "demo_nodemanagerdemo.h"
#include "nodemanagerroot.h"
#include "opcua_analogitemtype.h"
#include "opcua_dataitemtype.h"
#include "opcua_propertytype.h"
#include "opcuacnc_nodemanagercnc.h"
#include "userdatabase.h"

#include "misc.h"

#include <cstring>

// Include type headers

namespace
{

struct NodeUserData
	: UserDataBase
{
	NodeUserData(const adapter::xml_node_type *node);

	const adapter::xml_node_type *node;
};

}

// Namespace for the UA information model http://cs.hs-rm.de/mierswa_thesis/3AchsenEMCNCLaserSchneid/
namespace Demo {

/** Construction of the class NodeManagerDemo.
 */
NodeManagerDemo::NodeManagerDemo(
    std::shared_ptr<adapter::adapter> adapter,
    OpcUa_Boolean firesEvents,  //!< [in] Flag indicating if the NodeManager should activate the EventManager
    OpcUa_Int32 nHashTableSize) //!< [in] Size of the hash table. The default value is 10.007.<br>
                                //   The size can be smaller than the total number of nodes managed
                                //   by the node manager but it is better for the performance to avoid
                                //   collisions with a large size.
                                //   Prefered sizes are 1.009, 10.007, 100.003, 1.000.003, 10.000.019.
: NodeManagerDemoBase(firesEvents, nHashTableSize),
  adapter_(adapter)
{
}

/** Destroys the NodeManagerDemo object and all nodes managed by the node manager
 */
NodeManagerDemo::~NodeManagerDemo()
{
}

/** Finish start up in derived class after starting up base class.
 */
UaStatus NodeManagerDemo::afterStartUp()
{
	// The address space is created by the base class
	UaStatus ret = NodeManagerDemoBase::afterStartUp();

	if (!ret.isGood()) {
		return ret;
	}

	ServiceContext sCtx;
	NodeManagerRoot* root_nm = NodeManagerRoot::CreateRootNodeManager();
	OpcUa_NodeId nodeId;
	nodeId.IdentifierType = OpcUa_IdentifierType_Numeric;
	nodeId.NamespaceIndex = 0;
	nodeId.Identifier.Numeric = OpcUaId_RootFolder;
	OpcUa_NodeId refId;
	refId.IdentifierType = OpcUa_IdentifierType_Numeric;
	refId.NamespaceIndex = 0;
	refId.Identifier.Numeric = OpcUaId_HierarchicalReferences;
	OpcUa_ViewDescription desc;
	OpcUa_ViewDescription_Initialize(&desc);
	OpcUa_UInt32 nodeClassMask = OpcUa_NodeClass_Object | OpcUa_NodeClass_ObjectType | OpcUa_NodeClass_Variable;
	OpcUa_UInt32 browseMask = OpcUa_BrowseResultMask_TypeDefinition | OpcUa_BrowseResultMask_BrowseName;
	UaReferenceDescriptions refDesc;

	OpcUa_UInt16 cnc_namespace = OpcUaCnc::NodeManagerCNC::getTypeNamespace();

	auto addGMEvent = [this, root_nm, cnc_namespace](OpcUa_UInt16 tdNsIdx, OpcUa_UInt32 tdId, OpcUa_NodeId *addNode) {
		if (tdNsIdx != cnc_namespace ||
		    (tdId != OpcUaCncId_CncAxisListType &&
		     tdId != OpcUaCncId_CncChannelListType &&
		     tdId != OpcUaCncId_CncSpindleListType)) {
			return;
		}

		UaNode *foundNode = root_nm->getNode(*addNode);

		if (foundNode == NULL) {
			throw std::runtime_error("Node not managed by this server.");
		}

		const UaStatus retAdd = addUaReference(foundNode->nodeId(), OpcUaId_GeneralModelChangeEventType, OpcUaId_GeneratesEvent);

		if (!retAdd.isGood()) {
			foundNode->releaseReference();
			throw std::runtime_error("Unable to add GeneralModelChangeEventType reference to some nodes.");
		}

		foundNode->releaseReference();
	};

	std::function<void(adapter::xml_node_browse_path const &, OpcUa_NodeId *)> callback2;
	callback2 = [this, &addGMEvent, &callback2, &sCtx, root_nm, &desc, &refId, nodeClassMask, browseMask](adapter::xml_node_browse_path const &current_path, OpcUa_NodeId *browseNode) {
		//printf("current prefix %s\n", current_path.str().c_str());
#if 0
		UaString nsUri;
		UaStatus cbRet;

		cbRet = root_nm->getNamespaceUriFromIndex(browseNode->NamespaceIndex, nsUri);

		const std::string nsUriStr = nsUri.toUtf8();

		if (!cbRet.isGood()) {
			throw std::runtime_error("Unable to find namespace URI.");
		}

		UaNode *uaNode = root_nm->getNode(*browseNode);

		if (uaNode == NULL) {
			throw std::runtime_error("Can't fetch node while browsing.");
		}

		const adapter::xml_node_browse_name new_name = { nsUriStr, uaNode->browseName().toString().toUtf8() };
		adapter::xml_node_browse_path new_path = current_path;
		new_path.elements.push_back(new_name);

		uaNode->releaseReference();

		addGMEvent(browseNode->NamespaceIndex, browseNode->Identifier.Numeric, browseNode);

		adapter::xml_node_type *foundNode;

		adapter_->iterate_nodes([this, &foundNode, &current_path](adapter::xml_node_type const &passed_node) {
			if (passed_node.browse_path == current_path) {
				printf("found node: %s\n", current_path.str().c_str());
				return false;
			}
	
			return true;
		});
#endif

		BrowseContext bCtx(&desc, browseNode, 0, OpcUa_BrowseDirection_Forward, &refId, OpcUa_True, nodeClassMask, browseMask);
		UaReferenceDescriptions refDescs;
		const UaStatus res = root_nm->browse(sCtx, bCtx, refDescs);

		if (!res.isGood()) {
			return;
		}

		//if (refDescs.length() > 0) printf("looking at children\n");
		for (OpcUa_UInt32 i = 0; i < refDescs.length(); i++) {
			//const OpcUa_UInt16 bNameNs = refDescs[i].BrowseName.NamespaceIndex;
			//const std::string bNameStr = OpcUa_String_GetRawString(&refDescs[i].BrowseName.Name);
			const OpcUa_UInt16 tdNs = refDescs[i].TypeDefinition.NodeId.NamespaceIndex;
			const OpcUa_UInt32 tdId = refDescs[i].TypeDefinition.NodeId.Identifier.Numeric;

			if (tdNs == 0 && tdId == OpcUaId_ServerType) {
				continue;
			}

			OpcUa_NodeId *childNode = &refDescs[i].NodeId.NodeId;

			//const std::string nsUri = OpcUa_String_GetRawString(&refDescs[i].NodeId.NamespaceUri);
			UaStatus nsRet;
			UaString uaNsUri;
			nsRet = root_nm->getNamespaceUriFromIndex(refDescs[i].BrowseName.NamespaceIndex, uaNsUri);

			if (!nsRet.isGood()) {
				throw std::runtime_error("Unable to find namespace URI.");
			}

			const std::string nsUri = uaNsUri.toUtf8();

			UaNode *uaNode = root_nm->getNode(*childNode);

			if (uaNode == NULL) {
				throw std::runtime_error("Can't fetch node while browsing.");
			}



			const adapter::xml_node_browse_name new_name = { nsUri, uaNode->browseName().toString().toUtf8() };
			uaNode->releaseReference();
			if (new_name.name == "ActFeedrate") {
				printf("Feedrate NS: %s %d\n", new_name.ns.c_str(), childNode->Identifier.Numeric);
			}
			adapter::xml_node_browse_path new_path = current_path;
			new_path.elements.push_back(new_name);

			addGMEvent(childNode->NamespaceIndex, childNode->Identifier.Numeric, childNode);

			adapter_->iterate_nodes([this, childNode, &new_path](adapter::xml_node_type const &passed_node) {
				//printf("searching: %s %s\n", new_path.str().c_str(), passed_node.browse_path.str().c_str());
				if (passed_node.browse_path == new_path) {
					if (nodes_.find(&passed_node) != nodes_.end()) {
						throw std::runtime_error("Duplicate node.");
					}

					nodes_[&passed_node] = *childNode;
					return false;
				}

				return true;
			});


			//addGMEvent(tdNs, tdId, &refDescs[i].NodeId.NodeId);

			//printf("looking at child\n");
			callback2(new_path, childNode);
			//printf("looking at child done\n");
		}
		//if (refDescs.length() > 0) printf("looking at children done\n");
	};

	adapter::xml_node_browse_path current_browse_path;
	callback2(current_browse_path, &nodeId);

	//Misc::Timer elapsed_timer;

	//elapsed_timer.reset();

	auto callback = [this](adapter::xml_node_type const &node) {
		UaNodeId uaNodeId;

		try {
			uaNodeId = nodes_.at(&node);
		} catch (std::out_of_range const &) {
			char exception_string[1024];
			std::snprintf(exception_string, sizeof exception_string, "Node <%s> not present in address space.", node.browse_path.str().c_str());
			throw std::runtime_error(exception_string);
		}

		UaNode *uaNode = NodeManagerRoot::CreateRootNodeManager()->getNode(uaNodeId);

		if (uaNode == NULL) {
			throw std::runtime_error("Invalid node ID.");
		}

		try {
			setup_node(node, uaNode);
		} catch (std::exception const &exception) {
			uaNode->releaseReference();
			throw;
		}

		uaNode->releaseReference();

		return true;
	};
		
	adapter_->iterate_nodes(callback);

	//printf("elapsed: %g\n", elapsed_timer.elapsed());


	start();

	// Add custom start up code here
	return ret;
}

/** Start shut down in derived class before shutting down base class.
*/
UaStatus NodeManagerDemo::beforeShutDown()
{
    UaStatus ret;
    return ret;
}

void NodeManagerDemo::setup_node(adapter::xml_node_type const &node, UaNode *uaNode)
{
	//UaNode *uaNode = Misc::get_node_from_browse_path(node.browse_path);

	uaNode->setUserData(new NodeUserData(&node));

	if (node.name == "variable") {
		UaVariable *uaVar;

		try {
			uaVar = dynamic_cast<UaVariable *>(uaNode);
		} catch (std::bad_cast const &) {
			char exception_string[1024];
			std::snprintf(exception_string, sizeof exception_string, "Node <%s> is not a variable in address space.", node.browse_path.str().c_str());
			throw std::runtime_error(exception_string);
		}

		std::unique_ptr<adapter::xml_node_type> value_node;

		try {
			value_node.reset(new adapter::xml_node_type(node.children.at("value")[0]));
		} catch (std::out_of_range const &) {
			char exception_string[1024];
			std::snprintf(exception_string, sizeof exception_string, "Node <%s> is missing value child node.", node.browse_path.str().c_str());
			throw std::runtime_error(exception_string);
		}

		std::string kind;

		try {
			kind = value_node->attributes.at("kind");
		} catch (std::out_of_range const &) {
			char exception_string[1024];
			std::snprintf(exception_string, sizeof exception_string, "Node <%s> is missing kind attribute in value.", node.browse_path.str().c_str());
			throw std::runtime_error(exception_string);
		}

		if (kind == "fix") {
			if (value_node->value.empty()) {
				char exception_string[1024];
				std::snprintf(exception_string, sizeof exception_string, "Node <%s> has empty fixed value.", node.browse_path.str().c_str());
				throw std::runtime_error(exception_string);
			}
		}

		if (uaVar->typeDefinitionId() == OpcUaId_DataItemType) {
			OpcUa::DataItemType *diVar = dynamic_cast<OpcUa::DataItemType *>(uaVar);
			// could set value precision here
		} else if (uaVar->typeDefinitionId() == OpcUaId_PropertyType) {
			OpcUa::PropertyType *pVar = dynamic_cast<OpcUa::PropertyType *>(uaVar);
			// nothing can be added for property types
		} else if (uaVar->typeDefinitionId() == OpcUaId_AnalogItemType) {
			OpcUa::AnalogItemType *aiVar = dynamic_cast<OpcUa::AnalogItemType *>(uaVar);

			std::string unit;
			std::string range_start;
			std::string range_end;

			try {
				unit = value_node->attributes.at("unit");
				range_start = value_node->attributes.at("range_start");
				range_end = value_node->attributes.at("range_end");
			} catch (std::out_of_range const &) {
				char exception_string[1024];
				std::snprintf(exception_string, sizeof exception_string, "Node <%s> is missing attributes for AnalogItemType.", node.browse_path.str().c_str());
				throw std::runtime_error(exception_string);
			}

			aiVar->setEURange(UaRange(std::stod(range_start), std::stod(range_end)));

			UaEUInformation unit_info;

			if (unit == "mm/min") {
				unit_info = UaEUInformation::fromUnitId(UaEUInformation::EngineeringUnit_millimetre_per_minute);
			} else if (unit == "mm") {
				unit_info = UaEUInformation::fromUnitId(UaEUInformation::EngineeringUnit_millimetre);
			} else {
				char exception_string[1024];
				std::snprintf(exception_string, sizeof exception_string, "Node <%s> unit <%s> cannot be setup in UA server.", node.browse_path.str().c_str(), unit.c_str());
				throw std::runtime_error(exception_string);
			}

			aiVar->setEngineeringUnits(unit_info);
		} else {

			std::printf("Node <%s> has unknown data type <%s>.", node.browse_path.str().c_str(), uaVar->dataType().toFullString().toUtf8());
		}

		if (kind == "fix") {
			const UaNodeId dataType = uaVar->dataType();
			UaVariant variant;
			bool value_converted = true;

			if (dataType.namespaceIndex() == 0) {
				if (dataType.identifierNumeric() == OpcUaType_String) {
					variant.setString(value_node->value.c_str());
				} else if (dataType.identifierNumeric() == OpcUaType_Double) {
					variant.setDouble(std::stod(value_node->value));
				} else if (dataType.identifierNumeric() == OpcUaType_UInt32) {
					variant.setUInt32(static_cast<std::uint32_t>(std::stoi(value_node->value)));
				} else {
					value_converted = false;
				}
			} else {
				value_converted = false;
			}

			if (!value_converted) {
				char exception_string[1024];
				std::snprintf(exception_string, sizeof exception_string, "Node <%s> has unknown conversion for fix value <%s>", node.browse_path.str().c_str(), dataType.toFullString().toUtf8());
				throw std::runtime_error(exception_string);
			}
		} else if (kind == "fetcher") {
			std::string updated;

			try {
				updated = value_node->attributes.at("updated");
			} catch (std::out_of_range const &) {
				char exception_string[1024];
				std::snprintf(exception_string, sizeof exception_string, "Node <%s> is missing updated attribute in value.", node.browse_path.str().c_str());
				throw std::runtime_error(exception_string);
			}

			if (updated == "interval") {
				auto callback = [this, uaVar](adapter::xml_node_type const &passed_node, adapter::xml_node_fetch_info_type const &fetch_info) {
					set_variable_from_node(passed_node, uaVar, fetch_info);
				};

				adapter_->watch_node(node, callback);
			} else {
				char exception_string[1024];
				std::snprintf(exception_string, sizeof exception_string, "Node <%s> has unknown updated attribute in value.", node.browse_path.str().c_str());
				throw std::runtime_error(exception_string);
			}
		} else if (kind == "mirror") {
			std::string mirrored_browse_path;
			adapter::xml_node_type const *mirrored_node = NULL;

			try {
				mirrored_browse_path = value_node->attributes.at("browse_path");
			} catch (std::out_of_range const &) {
				char exception_string[1024];
				std::snprintf(exception_string, sizeof exception_string, "Node <%s> is missing browse_path attribute in value.", node.browse_path.str().c_str());
				throw std::runtime_error(exception_string);
			}

			auto callback = [&mirrored_node, &mirrored_browse_path](adapter::xml_node_type const &passed_node) {
				if (passed_node.browse_path.str() == mirrored_browse_path) {
					mirrored_node = &passed_node;
				}
			};

			if (mirrored_node == NULL) {
				char exception_string[1024];
				std::snprintf(exception_string, sizeof exception_string, "Node <%s> cannot be mirrored because <%s> is not setup.", node.browse_path.str().c_str(), mirrored_browse_path.c_str());
				throw std::runtime_error(exception_string);
			}

			std::printf("Node <%s> is mirrored to <%s>.\n", node.browse_path.str().c_str(), mirrored_browse_path.c_str());	
			mirrored_nodes_[mirrored_node] = &node;
		} else {
			char exception_string[1024];
			std::snprintf(exception_string, sizeof exception_string, "Node <%s> has unknown updated attribute in value.", node.browse_path.str().c_str());
			throw std::runtime_error(exception_string);
		}

		uaVar->setValueHandling(UaVariable_Value_CacheIsSource);
	} else {
		UaObject *uaObj = dynamic_cast<UaObject *>(uaNode);
	}

	std::printf("Node <%s> is set up.\n", node.browse_path.str().c_str());
}

void NodeManagerDemo::set_variable_from_node(adapter::xml_node_type const &node, UaVariable *variable, adapter::xml_node_fetch_info_type const &fetch_info)
{
	UaVariant variant;
	const UaNodeId dataType = variable->dataType();
	std::vector<std::uint8_t> const &bytes = fetch_info.bytes;

	switch (dataType.identifierNumeric()) {
		case OpcUaType_String: {
			const std::string string(bytes.begin(), bytes.end());
			variant.setString(string.c_str());
			break;
		}

		case OpcUaType_Double: {
			const std::string double_string(bytes.begin(), bytes.end());
			variant.setDouble(std::stod(double_string));
			break;
		}

		case OpcUaType_UInt32: {
			std::uint32_t number;
			std::memcpy(&number, bytes.data(), 4);
			variant.setUInt32(number);
			break;
		}

		default: {
			char exception_string[1024];
			std::snprintf(exception_string, sizeof exception_string, "Cannot set value for node <%s>.", node.browse_path.str().c_str());
			throw std::runtime_error(exception_string);
		}
	}

	UaDataValue dv;
	dv.setValue(variant, OpcUa_True, OpcUa_True);

	variable->setValue(NULL, dv, OpcUa_False);

	printf("setting variable from node: %s\n", node.browse_path.str().c_str());
	
}

void NodeManagerDemo::run()
{
	adapter_->run();
}

} // End namespace for the UA information model http://cs.hs-rm.de/mierswa_thesis/3AchsenEMCNCLaserSchneid/

namespace
{

NodeUserData::NodeUserData(const adapter::xml_node_type *passed_node)
	: node(passed_node)
{
}

}
