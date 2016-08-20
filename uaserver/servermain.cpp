#include "opcserver.h"
#include "uaplatformlayer.h"
#include "uathread.h"
#if SUPPORT_XML_PARSER
  #include "xmldocument.h"
#endif
#include "shutdown.h"


#include "basenodes.h"
#include "nodemanagernodesetxml.h"
#include "opcua_acknowledgeableconditiontype.h"
#include "opcua_alarmconditiontype.h"
#include "opcua_basemodelchangeeventtypedata.h"
#include "opcua_conditiontype.h"
#include "opcua_discretealarmtype.h"
#include "opcuatypes.h"
#include "instancefactory.h"

#include "uagenericnodes.h"

#include <adapter/adapter.h>

#include "opcuadi_nodemanagerdevices.h"
#include "gen_models/opcuacnc_nodemanagercnc.h"
#include "gen_models/demo_nodemanagerdemo.h"

#include "xmldocument.h"

#include <stdexcept>
#include <string>

#include <wrapper/wrapper.h>

static adapter::xml_node_type add_node(UaXmlDocument &document, std::string const &name, adapter::xml_node_type *parent = NULL);

class MyNodeManagerNodeSetXmlCreator;
//static OpcUa_UInt32 currentNodeId = 5501;

class MyXmlUaNodeFactoryManager
	: public XmlUaNodeFactoryManager
{
	UA_DISABLE_COPY(MyXmlUaNodeFactoryManager);

public:
	MyXmlUaNodeFactoryManager()
		: XmlUaNodeFactoryManager()
	{
	}

	virtual ~MyXmlUaNodeFactoryManager()
	{
	}

	UaObject *createObject(UaBase::Object *pObject, NodeManagerConfig *pNodeConfig, UaMutexRefCounted *pSharedMutex = NULL)
	{
		printf("Object create: %s\n", pObject->nodeId().toFullString().toUtf8());

		return XmlUaNodeFactoryManager::createObject(pObject, pNodeConfig, pSharedMutex);
	}
};

#if 0
class MyNodeManager
	: public NodeManagerBase
{
	UA_DISABLE_COPY(MyNodeManager);
	MyNodeManager();

public:
	MyNodeManager(NodeManagerNodeSetXml *pNmCNC, NodeManagerNodeSetXml *pNmEMCNC)
		: NodeManagerBase("urn:local", OpcUa_True, 10007),
		  m_pNmCNC(pNmCNC),
		  m_pNmEMCNC(pNmEMCNC)
	{
	}

	virtual ~MyNodeManager()
	{
	}

	template <class T>
	UaNode *getCNCNode(T const &t)
	{
		OpcUa_UInt16 nsIdx = m_pNmCNC->getNameSpaceIndex();

		return m_pNmCNC->getNode(UaNodeId(t, nsIdx));
	}

	template <class T>
	UaNode *getEMCNCNode(T const &t)
	{
		OpcUa_UInt16 nsIdx = m_pNmEMCNC->getNameSpaceIndex();

		return m_pNmEMCNC->getNode(UaNodeId(t, nsIdx));
	}


	UaNode *createEMCNCMachine()
	{
		OpcUa_UInt16 nsIdx = getNameSpaceIndex();

		UaGenericObject *pMachine = new UaGenericObject(
			UaNodeId(currentNodeId++, nsIdx), /* id */
			UaQualifiedName("MyCNCMachine", nsIdx), /* browse */
			UaLocalizedText("en", "MyCNCMachine"), /* display */
			UaLocalizedText("en", "A CNC Machine"), /* description */
			getEMCNCNode("EckelmannCNCMachineType")->nodeId(), /* type ID */
			OpcUa_False /* event notifier */);

		return pMachine;
	}

	UaNode *createCncInterface()
	{
		OpcUa_UInt16 nsIdx = getNameSpaceIndex();
		UaNode *pObject;
		UaNode *pInterface;
		UaStatus addStatus;

		pObject = new UaGenericObject(
			UaNodeId(currentNodeId++, nsIdx), /* id */
			UaQualifiedName("VDWCncInterface", nsIdx), /* browse */
			UaLocalizedText("en", "VDWCncInterface"), /* display */
			UaLocalizedText("en", "The VDW CNC Interface"), /* description */
			getCNCNode(1001)->nodeId(), /* type ID */
			OpcUa_False /* event notifier */);

		pInterface = pObject;

		{
			UaReferenceLists *pRefLists = getCNCNode(1001)->getUaReferenceLists();
			const UaReference *pRef = pRefLists->pTargetNodes();

			while (pRef != 0) {
				UaNode *pTargetNode = pRef->pTargetNode();
				const UaNodeId refId = pRef->referenceTypeId();
				pObject = 0;

				if (pTargetNode->nodeClass() == OpcUa_NodeClass_Object) {
					pObject = new OpcUa::BaseObjectType(
						UaNodeId(currentNodeId++, nsIdx), /* id */
						(UaObject *)pTargetNode, /* instance declaration */
						this /* node config */);
				} else if (pTargetNode->nodeClass() == OpcUa_NodeClass_Variable) {
					pObject = new OpcUa::BaseVariableType(
						pInterface, /* parent */
						(UaVariable *)pTargetNode, /* instance declaration */
						this /* node config */);
				}

				if (pObject != 0) {
					addStatus = addNodeAndReference(pInterface->getUaReferenceLists(), pObject->getUaReferenceLists(), refId);
					printf("%s\n", addStatus.toString().toUtf8());
					UA_ASSERT(addStatus.isGood());
				}

				pRef = ((UaReference *)pRef)->pNextForwardReference();
			}
		}

		return pInterface;
	}

	UaNode *createCncChannel()
	{
		OpcUa_UInt16 nsIdx = getNameSpaceIndex();
		UaNode *pObject;
		UaNode *pChannel;
		UaStatus addStatus;

		pObject = new UaGenericObject(
			UaNodeId(currentNodeId++, nsIdx), /* id */
			UaQualifiedName("VDWCncChannel1", nsIdx), /* browse */
			UaLocalizedText("en", "VDWCncChannel1"), /* display */
			UaLocalizedText("en", "A VDW CNC Channel"), /* description */
			getCNCNode(1006)->nodeId(), /* type ID */
			OpcUa_False /* event notifier */);

		pChannel = pObject;

		{
			UaReferenceLists *pRefLists = getCNCNode(1006)->getUaReferenceLists();
			const UaReference *pRef = pRefLists->pTargetNodes();

			while (pRef != 0) {
				UaNode *pTargetNode = pRef->pTargetNode();
				const UaNodeId refId = pRef->referenceTypeId();
				pObject = 0;

				if (refId != OpcUaId_GeneratesEvent) {
					if (pTargetNode->nodeClass() == OpcUa_NodeClass_Object) {
						pObject = new OpcUa::BaseObjectType(
							UaNodeId(currentNodeId++, nsIdx), /* id */
							(UaObject *)pTargetNode, /* instance declaration */
							this /* node config */);
					} else if (pTargetNode->nodeClass() == OpcUa_NodeClass_Variable) {
						pObject = new OpcUa::BaseVariableType(
							pChannel, /* parent */
							(UaVariable *)pTargetNode, /* instance declaration */
							this /* node config */);
					}
				}

				if (pObject != 0) {
					addStatus = addNodeAndReference(pChannel->getUaReferenceLists(), pObject->getUaReferenceLists(), refId);
					printf("%s\n", addStatus.toString().toUtf8());
					UA_ASSERT(addStatus.isGood());
				}

				pRef = ((UaReference *)pRef)->pNextForwardReference();
			}
		}

		return pChannel;
	}

	virtual UaStatus afterStartUp()
	{
		OpcUa::GeneralModelChangeEventTypeData::createTypes();

		UaStatus addStatus;
		UaNode *pObject;
		UaNode *pMachine;

		{
			pObject = createEMCNCMachine();
			addStatus = addNodeAndReference(OpcUaId_ObjectsFolder, pObject->getUaReferenceLists(), OpcUaId_Organizes);
			UA_ASSERT(addStatus.isGood());

			pMachine = pObject;
		}

		{
			pObject = createCncInterface();
			addStatus = addNodeAndReference(pMachine->getUaReferenceLists(), pObject->getUaReferenceLists(), OpcUaId_HasComponent);
			UA_ASSERT(addStatus.isGood());
		}

		{
			UaNode *nodeToAddTo = getNode(UaNodeId(5505, getNameSpaceIndex()));
			pObject = createCncChannel();
			addStatus = addNodeAndReference(nodeToAddTo->getUaReferenceLists(), pObject->getUaReferenceLists(), OpcUaId_HasComponent);
			UA_ASSERT(addStatus.isGood());
		}

#if 0
		{
			UaReferenceLists *pRefLists = getCNCNode(1001)->getUaReferenceLists();
			const UaReference *pRef = pRefLists->pTargetNodes();

			while (pRef != 0) {
				UaNode *pTargetNode = pRef->pTargetNode();
				const UaNodeId refId = pRef->referenceTypeId();

				if (pTargetNode->nodeClass() == OpcUa_NodeClass_Object) {
					UaObject *pObjectDecl = (UaObject *)pTargetNode;
					OpcUa::BaseObjectType *pObject = new OpcUa::BaseObjectTypeGeneric(currentNodeId++, pObjectDecl, this);

					addStatus = addNodeAndReference(pInterface, pObject, refId);
					printf("%s\n", addStatus.toString().toUtf8());
				}
				
				//printf("%p %p %d %s\n", pRef, pTargetNode, pTargetNode->nodeClass(), pTargetNode->browseName().toString().toUtf8());
				pRef = ((UaReference *)pRef)->pNextForwardReference();
			}
		}
#endif

/*
		{
			UaNodeId id = UaNodeId(5502, nsIdx);
			UaQualifiedName browseName = UaQualifiedName("VDWCncInterface", nsIdx);
			UaLocalizedText displayName = UaLocalizedText("en", "VDWCncInterface");
			UaLocalizedText description = UaLocalizedText("en", "VDW CNC Interface");
			UaNode *pNode = getCNCNode(1001);

printf("%d\n", pNode->nodeClass());
			UaNodeId typeId = pNode->nodeId();
			OpcUa_Byte eventNotifier = OpcUa_False;

			pObject = new UaGenericObject(id, browseName, displayName, description, typeId, eventNotifier);

			addStatus = pNmRoot->addNodeAndReference(pMachNode->nodeId(), pObject, OpcUaId_HasComponent);
			UA_ASSERT(addStatus.isGood());
		}
		ServiceContext *ctx = new ServiceContext();
		UaNodeId parentId = OpcUaId_ObjectsFolder;
		UaNodeId refId = OpcUaId_Organizes;
		UaNodeId reqId = UaNodeId(5001, nsIdx);
		UaQualifiedName browseName = UaQualifiedName("VDWCncInterface", nsIdx);
		OpcUa_NodeClass nodeClass = OpcUa_NodeClass_Object;
		UaExtensionObject ext = UaExtensionObject();
		UaNodeId typeId = getCNCNode(1001)->nodeId();
		UaNodeId addedId;
		addStatus = pNmRoot->addNode(*ctx, parentId, refId, reqId, browseName, nodeClass, ext, typeId, addedId);
		printf("%s\n", addStatus.toString().toUtf8());
		UA_ASSERT(addStatus.isGood());

		printf("%s\n", addedId.toFullString().toUtf8());
*/
/*
    virtual UaStatus addNode(
        const ServiceContext&    serviceContext,
        const UaNodeId&          parentNodeId,
        const UaNodeId&          referenceTypeId,
        const UaNodeId&          requestedNewNodeId,
        const UaQualifiedName&   browseName,
        OpcUa_NodeClass          nodeClass,
        const UaExtensionObject& nodeAttributes,
        const UaNodeId&          typeDefinition,
        UaNodeId&                addedNodeId);
*/

		return UaStatus();
	}

	virtual UaStatus beforeShutDown()
	{
		return UaStatus();
	}

private:
	NodeManagerNodeSetXml *m_pNmCNC;
	NodeManagerNodeSetXml *m_pNmEMCNC;
};
#endif

class MyNodeManagerNodeSetXml
	: public NodeManagerNodeSetXml
{
	UA_DISABLE_COPY(MyNodeManagerNodeSetXml);
	MyNodeManagerNodeSetXml();

public:
	MyNodeManagerNodeSetXml(MyNodeManagerNodeSetXmlCreator *pCreator, const UaString& sNamespaceUri)
		: NodeManagerNodeSetXml(sNamespaceUri),
		  m_pCreator(pCreator),
		  m_sNamespaceUri(sNamespaceUri)
	{
	}

	virtual ~MyNodeManagerNodeSetXml()
	{
	}

	virtual UaStatus afterStartUp()
	{
		return UaStatus();
	}

	virtual UaStatus beforeShutDown()
	{
		return UaStatus();
	}

	//virtual void allNodesAndReferencesCreated();

#if 0
	virtual void objectCreated(UaObject *pNewNode, UaBase::Object *pObject)
	{
		printf("%s\n", pNewNode->nodeId().toFullString().toUtf8());
	}

	virtual void objectTypeCreated(UaObjectType *pNewNode, UaBase::ObjectType *pObjectType)
	{
		printf("%s\n", pNewNode->nodeId().toFullString().toUtf8());
	}
#endif

private:
	MyNodeManagerNodeSetXmlCreator *m_pCreator;
	UaString m_sNamespaceUri;
};

class MyNodeManagerNodeSetXmlCreator
	: public NodeManagerNodeSetXmlCreator
{
	UA_DISABLE_COPY(MyNodeManagerNodeSetXmlCreator);

public:
	MyNodeManagerNodeSetXmlCreator(OpcServer *pServer)
		: m_pServer(pServer)
	{
	}

	virtual ~MyNodeManagerNodeSetXmlCreator()
	{
	}

	virtual NodeManagerNodeSetXml *createNodeManager(const UaString& sNamespaceUri)
	{
		NodeManagerNodeSetXml *pXmlManager;

		pXmlManager = new MyNodeManagerNodeSetXml(this, sNamespaceUri);

	/*
		if (sNamespaceUri == UaString("http://opcfoundation.org/UA/CNC/")) {
			m_pNmCNC = pXmlManager;
		} else if (sNamespaceUri == UaString("http://eckelmann.de/EckelmannCNC/")) {
			m_pNmEMCNC = pXmlManager;
		}
	*/

		return pXmlManager;
	}

	//void allNodesAndReferencesCreated()
	void setup()
	{
	/*
		if (m_pNmCNC != 0 && m_pNmEMCNC != 0) {
			m_pNmMain = new MyNodeManager(m_pNmCNC, m_pNmEMCNC);

			m_pServer->addNodeManager(m_pNmMain);
		}
	*/
	}

private:
	//NodeManagerNodeSetXml *m_pNmCNC;
	//NodeManagerNodeSetXml *m_pNmEMCNC;
	//NodeManagerBase *m_pNmMain;
	OpcServer *m_pServer;
};

class MyVariable : public UaBase::Variable
{
public:
	MyVariable(const UaNodeId &nodeId)
		: UaBase::Variable(nodeId)
	{
	}
};

class MyBaseNodeFactory : public UaBase::BaseNodeFactory
{
public:
	virtual UaBase::Object* createObject(const UaNodeId &nodeId) const
	{
printf("create object: %s\n", nodeId.toFullString().toUtf8());
		return new UaBase::Object(nodeId);
	}

	virtual MyVariable* createVariable(const UaNodeId &nodeId) const
	{
		return new MyVariable(nodeId);
	}
};


int OpcServerMain(const char* szAppPath)
{
    int ret = 0;

    //- Initialize the environment --------------
#if SUPPORT_XML_PARSER
    // Initialize the XML Parser
    UaXmlDocument::initParser();
#endif
    // Initialize the UA Stack platform layer
    ret = UaPlatformLayer::init();
    //-------------------------------------------

    if ( ret == 0 )
    {
        // Create configuration file name
#ifdef SERVER_ALTERNATE_CONFIG_PATH
	#if SUPPORT_XML_PARSER
		const UaString sConfigFileName(SERVER_ALTERNATE_CONFIG_PATH "/ServerConfig.xml");
	#else
		const UaString sConfigFileName(SERVER_ALTERNATE_CONFIG_PATH "/ServerConfig.ini");
	#endif
#else
		UaString sConfigFileName(szAppPath);

	#if SUPPORT_XML_PARSER
        sConfigFileName += "/ServerConfig.xml";
	#else
        sConfigFileName += "/ServerConfig.ini";
	#endif
#endif

        //- Start up OPC server ---------------------
        // This code can be integrated into a startup
        // sequence of the application where the
        // OPC server should be integrated
        //-------------------------------------------
        // Create and initialize server object
        OpcServer* pServer = new OpcServer;
		pServer->setServerConfig(sConfigFileName, szAppPath);

	NodeManager *manager;
	UaServerApplicationModule *module;

        manager = new OpcUaDi::NodeManagerDevices();
        pServer->addNodeManager(manager);

	manager = new OpcUaCnc::NodeManagerCNC(OpcUa_True);
	pServer->addNodeManager(manager);

{
#ifdef ADAPTER_ALTERNATE_CONFIG_PATH
	const UaString sAdapterConfigFileName(ADAPTER_ALTERNATE_CONFIG_PATH "/AdapterConfig.xml");
#else
	const UaString sAdapterConfigFileName("AdapterConfig.xml");
#endif

	UaXmlDocument adapterConfigDocument(sAdapterConfigFileName.toUtf8());
	adapter::xml_node_map_type adapterConfigNodes;

	while (true) {
		const char *name = adapterConfigDocument.getNodeName();

		if (name == NULL) {
			break;
		}

		const adapter::xml_node_type node = add_node(adapterConfigDocument, name);

		adapterConfigNodes[name].push_back(node);

		const int next = adapterConfigDocument.getNextSibling();

		if (next == -1) {
			break;
		}
	}

	std::shared_ptr<adapter::adapter> adapter(new wrapper::wrapper(adapterConfigNodes));
	manager = new Demo::NodeManagerDemo(adapter, OpcUa_True);
	pServer->addNodeManager(manager);
}

#if defined(USE_XML_1)
#if SUPPORT_XML_PARSER
        // We create our own BaseNode factory to create the user data from XML
        UaBase::BaseNodeFactory* pBaseNodeFactory = new MyBaseNodeFactory;

        // We create our own NodeManager creator to instantiate our own NodeManager
        MyNodeManagerNodeSetXmlCreator* pNodeManagerCreator = new MyNodeManagerNodeSetXmlCreator(pServer);

	MyXmlUaNodeFactoryManager *pFactoryManager = new MyXmlUaNodeFactoryManager();

	{
		//UaString sNodesetFile(UaString("%1/vdw_cnc.xml").arg(szAppPath));
		const UaString sNodesetFile(UaString("%1/vdw_cnc.xml").arg("/home/impulze"));
		module = new UaNodeSetXmlParserUaNode(sNodesetFile, pNodeManagerCreator, pBaseNodeFactory, pFactoryManager);
		pServer->addModule(module);
	}


#if 0
	{
		//UaString sNodesetFile(UaString("%1/eckelmann_cnc.xml").arg(szAppPath));
		UaString sNodesetFile(UaString("%1/eckelmann_cnc.xml").arg("/home/impulze"));
		pXmlParser = new UaNodeSetXmlParserUaNode(sNodesetFile, pNodeManagerCreator, pBaseNodeFactory, pFactoryManager);
		pServer->addModule(pXmlParser);
	}
#endif

/*
	{
		UaString sNodesetFile(UaString("%1/my.xml").arg(szAppPath));
		pXmlParser = new UaNodeSetXmlParserUaNode(sNodesetFile, pNodeManagerCreator, pBaseNodeFactory, NULL);
		pServer->addModule(pXmlParser);
	}
*/
#endif
#endif

        // Start server object
        ret = pServer->start();
        if ( ret != 0 )
        {
            delete pServer;
            pServer = 0;
        }
        //-------------------------------------------


        if ( ret == 0 )
        {
#if !defined(USE_XML_1)
#if 0
            {
                  static int created = 0;

                  if (created == 0) {
                      pNodeManagerCreator->setup();
                      created = 1;
                  }
            }
#endif
#endif

            printf("***************************************************\n");
            printf(" Press %s to shut down server\n", SHUTDOWN_SEQUENCE);
            printf("***************************************************\n");
            // Wait for user command to terminate the server thread.
            while ( ShutDownFlag() == 0 )
            {
                UaThread::msleep(1000);
            }
            printf("***************************************************\n");
            printf(" Shutting down server\n");
            printf("***************************************************\n");


            //- Stop OPC server -------------------------
            // This code can be integrated into a shutdown
            // sequence of the application where the
            // OPC server should be integrated
            //-------------------------------------------
            // Stop the server and wait three seconds if clients are connected
            // to allow them to disconnect after they received the shutdown signal
            pServer->stop(3, UaLocalizedText("", "User shutdown"));
            delete pServer;
            pServer = NULL;
            //-------------------------------------------
        }
    }

    //- Clean up the environment --------------
    // Clean up the UA Stack platform layer
    UaPlatformLayer::cleanup();
#if SUPPORT_XML_PARSER
    // Clean up the XML Parser
    UaXmlDocument::cleanupParser();
#endif
    //-------------------------------------------

    return ret;
}

#ifdef _WIN32_WCE
int WINAPI WinMain( HINSTANCE, HINSTANCE, LPWSTR, int)
#else
int main(int, char*[])
#endif
{
    int ret = 0;

    RegisterSignalHandler();

    // Extract application path
    char* pszAppPath = getAppPath();

    //-------------------------------------------
    // Call the OPC server main method
    ret = OpcServerMain(pszAppPath);
    //-------------------------------------------

    if ( pszAppPath ) delete [] pszAppPath;

    return ret;
}

/*
void MyNodeManagerNodeSetXml::allNodesAndReferencesCreated()
{
	m_pCreator->allNodesAndReferencesCreated();
}
*/

adapter::xml_node_type add_node(UaXmlDocument &document, std::string const &name, adapter::xml_node_type *parent)
{
	UaXmlAttributeList foundAttributes;
	adapter::xml_attribute_map_type attributes;

	document.getAttributes(foundAttributes);

	for (auto const &foundAttribute: foundAttributes) {
		// TODO: this is not document and it isn't rightly implemented
		attributes[foundAttribute.getValue()] = foundAttribute.getName();
	}

	adapter::xml_node_browse_path browse_path;

	if (name == "object" || name == "variable") {
		const std::string ns = document.getNamespace();
		const std::string browse_name = attributes.at("browse_name");
		const adapter::xml_node_browse_name param_browse_name = { ns, browse_name };

		if (parent != NULL) {
			browse_path = parent->browse_path;
		}

		browse_path.elements.push_back(param_browse_name);
	}

	adapter::xml_node_type node(name, browse_path);

	node.attributes = attributes;
	node.ns = document.getNamespace();

	UaXmlValue value;
	document.getContent(value);
	node.value = value.pValue();

	const int child_result = document.getFirstChild();

	if (child_result == -1) {
		return node;
	}

	while (true) {
		const char *child_name = document.getNodeName();

		if (child_name == NULL) {
			break;
		}

		adapter::xml_node_type child_node = add_node(document, child_name, &node);

		node.children[child_name].push_back(child_node);

		const int next = document.getNextSibling();

		if (next == -1) {
			break;
		}
	}

	document.getParentNode();
	return node;
}

