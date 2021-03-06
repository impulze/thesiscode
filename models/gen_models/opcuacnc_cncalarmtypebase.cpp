/******************************************************************************
** opcuacnc_cncalarmtypebase.cpp
**
**   **************************** auto-generated ****************************
**     This code was generated by a tool: UaModeler
**     Runtime Version: 1.5.0, using C++ OPC UA SDK 1.5.0 template (version 4)
**
**     Changes to this file may cause incorrect behavior and will be lost if
**     the code is regenerated.
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
** Project: C++ OPC Server SDK information model for namespace http://opcfoundation.org/UA/CNC/
**
** Description: OPC Unified Architecture Software Development Kit.
**
** Created: 19.08.2016
**
******************************************************************************/

#include "opcuacnc_cncalarmtypebase.h"
#include "opcuacnc_cncalarmtype.h"
#include "uagenericnodes.h"
#include "nodemanagerroot.h"
#include "eventmanageruanode.h"
#include "methodhandleuanode.h"
#include "instancefactory.h"
#include "opcuacnc_nodemanagercnc.h"

// Namespace for the UA information model http://opcfoundation.org/UA/CNC/
namespace OpcUaCnc {

bool CncAlarmTypeBase::s_typeNodesCreated = false;
OpcUa::PropertyType* CncAlarmTypeBase::s_pAlarmIdentifier = NULL;
OpcUa::PropertyType* CncAlarmTypeBase::s_pAuxParameters = NULL;
OpcUa::PropertyType* CncAlarmTypeBase::s_pHelpSource = NULL;
std::map<OpcUa_UInt32, OpcUa_UInt32> CncAlarmTypeBase::s_CncAlarmTypeDataFields;

/** Constructs an instance of the CncAlarmType condition type using an instance declaration node as base.
*/
CncAlarmTypeBase::CncAlarmTypeBase(const UaNodeId& nodeId, UaObject* pInstanceDeclarationObject, NodeManagerConfig* pNodeConfig, const UaNodeId& sourceNode, const UaString& sourceName, UaMutexRefCounted* pSharedMutex)
: OpcUa::DiscreteAlarmType(nodeId, pInstanceDeclarationObject, pNodeConfig, sourceNode, sourceName, pSharedMutex)
{
    createChildren();
}

/** Constructs an instance of the CncAlarmType condition type.
*/
CncAlarmTypeBase::CncAlarmTypeBase(const UaNodeId& nodeId, const UaString& name, OpcUa_UInt16 browseNameNameSpaceIndex, NodeManagerConfig* pNodeConfig, const UaNodeId& sourceNode, const UaString& sourceName, UaMutexRefCounted* pSharedMutex)
: OpcUa::DiscreteAlarmType(nodeId, name, browseNameNameSpaceIndex, pNodeConfig, sourceNode, sourceName, pSharedMutex)
{
    createChildren();
}

/**  Constructs an instance of the class CncAlarmTypeBase with all components.
*/
CncAlarmTypeBase::CncAlarmTypeBase(
    UaBase::Object*    pBaseNode,    //!< [in] The "tree" of nodes that specifies the attributes of the instance and its children.
    XmlUaNodeFactoryManager* pFactory, //!< [in] The factory to create the children.
    NodeManagerConfig* pNodeConfig,  //!< [in] Interface pointer to the NodeManagerConfig interface used to add and delete node and references in the address space.
    UaMutexRefCounted* pSharedMutex) //!< [in] Shared mutex object used to synchronize access to the variable. Can be NULL if no shared mutex is provided.
: OpcUa::DiscreteAlarmType(pBaseNode, pFactory, pNodeConfig, pSharedMutex)
{
    initialize();

    m_pAlarmIdentifier = NULL;
    m_pAuxParameters = NULL;
    m_pHelpSource = NULL;

    UaStatus      addStatus;
    std::list<UaBase::BaseNode*> lstReferencedNodes = pBaseNode->hierarchicallyReferencedNodes();
    for (std::list<UaBase::BaseNode*>::const_iterator it = lstReferencedNodes.begin(); it != lstReferencedNodes.end(); it++)
    {
        UaBase::BaseNode *pChild = *it;
        if (pChild->browseName() == UaQualifiedName("AlarmIdentifier", OpcUaCnc::NodeManagerCNC::getTypeNamespace()))
        {
            if (!m_pAlarmIdentifier)
            {
                m_pAlarmIdentifier = (OpcUa::PropertyType*) pFactory->createVariable((UaBase::Variable*)*it, pNodeConfig, pSharedMutex);
                addStatus = pNodeConfig->addNodeAndReference(this, m_pAlarmIdentifier, OpcUaId_HasProperty);
                UA_ASSERT(addStatus.isGood());
            }
        }
        else if (pChild->browseName() == UaQualifiedName("AuxParameters", OpcUaCnc::NodeManagerCNC::getTypeNamespace()))
        {
            if (!m_pAuxParameters)
            {
                m_pAuxParameters = (OpcUa::PropertyType*) pFactory->createVariable((UaBase::Variable*)*it, pNodeConfig, pSharedMutex);
                addStatus = pNodeConfig->addNodeAndReference(this, m_pAuxParameters, OpcUaId_HasProperty);
                UA_ASSERT(addStatus.isGood());
            }
        }
        else if (pChild->browseName() == UaQualifiedName("HelpSource", OpcUaCnc::NodeManagerCNC::getTypeNamespace()))
        {
            if (!m_pHelpSource)
            {
                m_pHelpSource = (OpcUa::PropertyType*) pFactory->createVariable((UaBase::Variable*)*it, pNodeConfig, pSharedMutex);
                addStatus = pNodeConfig->addNodeAndReference(this, m_pHelpSource, OpcUaId_HasProperty);
                UA_ASSERT(addStatus.isGood());
            }
        }
    }
    UA_ASSERT(m_pAlarmIdentifier);
}

void CncAlarmTypeBase::initialize()
{
    OpcUa_Int16 nsTypeIdx = NodeManagerCNC::getTypeNamespace();
    m_EventTypeId.setNodeId(OpcUaCncId_CncAlarmType, nsTypeIdx);

    m_FieldValues.create(3);

    UaVariant   eventTypeValue;
    UaDataValue eventTypeDataValue;
    eventTypeValue.setNodeId(m_EventTypeId);
    eventTypeDataValue.setValue(eventTypeValue, OpcUa_True, OpcUa_True);
    m_pEventTypeProperty->setValue(NULL, eventTypeDataValue, OpcUa_False);

    if ( s_typeNodesCreated == false )
    {
        createTypes();
    }
}

void CncAlarmTypeBase::createChildren()
{
    initialize();
    UaStatus    addStatus;

    // Mandatory variable AlarmIdentifier
    m_pAlarmIdentifier = new OpcUa::PropertyType(this, s_pAlarmIdentifier, m_pNodeConfig, m_pSharedMutex);
    addStatus = m_pNodeConfig->addNodeAndReference(this, m_pAlarmIdentifier, OpcUaId_HasProperty);
    UA_ASSERT(addStatus.isGood());

    // Optional variable AuxParameters
    m_pAuxParameters = NULL;
    // Optional variable HelpSource
    m_pHelpSource = NULL;

}

/** Destroys the CncAlarmType object
*/
CncAlarmTypeBase::~CncAlarmTypeBase()
{
}

/** Create the related type nodes
*/
void CncAlarmTypeBase::createTypes()
{
    if ( s_typeNodesCreated == false )
    {
        s_typeNodesCreated = true;

        // Check if supertype is already created
        OpcUa::DiscreteAlarmType::createTypes();

        registerEventFields();

        UaStatus      addStatus;
        UaVariant     defaultValue;
        NodeManagerRoot* pNodeManagerRoot = NodeManagerRoot::CreateRootNodeManager();
        OpcUa_Int16 nsTypeIdx = NodeManagerCNC::getTypeNamespace();
        OpcUa_Int16 nsSuperTypeIdx = NodeManagerRoot::getTypeNamespace();
        NodeManagerConfig* pTypeNodeConfig = pNodeManagerRoot->getNodeManagerByNamespace(nsTypeIdx)->getNodeManagerConfig();

        OpcUa::GenericObjectType* pObjectType;
        pObjectType = new OpcUa::GenericObjectType(
            UaNodeId(OpcUaCncId_CncAlarmType, nsTypeIdx),
            UaQualifiedName("CncAlarmType", nsTypeIdx),
            UaLocalizedText("", "CncAlarmType"),
            UaLocalizedText("", ""),
            OpcUa_False,
            &CncAlarmType::clearStaticMembers);
        addStatus = pTypeNodeConfig->addNodeAndReference(UaNodeId(OpcUaId_DiscreteAlarmType, nsSuperTypeIdx), pObjectType, OpcUaId_HasSubtype);
        UA_ASSERT(addStatus.isGood());


        // Mandatory variable AlarmIdentifier
        defaultValue.setString("");
        s_pAlarmIdentifier = new OpcUa::PropertyType(UaNodeId(OpcUaCncId_CncAlarmType_AlarmIdentifier, nsTypeIdx), "AlarmIdentifier", OpcUaCnc::NodeManagerCNC::getTypeNamespace(), defaultValue, 1, pTypeNodeConfig);
        s_pAlarmIdentifier->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
        s_pAlarmIdentifier->setValueRank(-1);
        addStatus = pTypeNodeConfig->addNodeAndReference(pObjectType, s_pAlarmIdentifier, OpcUaId_HasProperty);
        UA_ASSERT(addStatus.isGood());

        // Optional variable AuxParameters
        defaultValue.setString("");
        s_pAuxParameters = new OpcUa::PropertyType(UaNodeId(OpcUaCncId_CncAlarmType_AuxParameters, nsTypeIdx), "AuxParameters", OpcUaCnc::NodeManagerCNC::getTypeNamespace(), defaultValue, 1, pTypeNodeConfig);
        s_pAuxParameters->setModellingRuleId(OpcUaId_ModellingRule_Optional);
        s_pAuxParameters->setValueRank(-1);
        addStatus = pTypeNodeConfig->addNodeAndReference(pObjectType, s_pAuxParameters, OpcUaId_HasProperty);
        UA_ASSERT(addStatus.isGood());

        // Optional variable HelpSource
        defaultValue.setString("");
        s_pHelpSource = new OpcUa::PropertyType(UaNodeId(OpcUaCncId_CncAlarmType_HelpSource, nsTypeIdx), "HelpSource", OpcUaCnc::NodeManagerCNC::getTypeNamespace(), defaultValue, 1, pTypeNodeConfig);
        s_pHelpSource->setModellingRuleId(OpcUaId_ModellingRule_Optional);
        s_pHelpSource->setValueRank(-1);
        addStatus = pTypeNodeConfig->addNodeAndReference(pObjectType, s_pHelpSource, OpcUaId_HasProperty);
        UA_ASSERT(addStatus.isGood());

    }
}

/** Clear the static members of the class
*/
void CncAlarmTypeBase::clearStaticMembers()
{
    s_typeNodesCreated = false;

    s_pAlarmIdentifier = NULL;
    s_pAuxParameters = NULL;
    s_pHelpSource = NULL;
    s_CncAlarmTypeDataFields.clear();
}

/** Returns the type definition NodeId for the CncAlarmType
*/
UaNodeId CncAlarmTypeBase::typeDefinitionId() const
{
    UaNodeId ret(OpcUaCncId_CncAlarmType, NodeManagerCNC::getTypeNamespace());
    return ret;
}

/** Registers all event type fields with the EventManagerBase.
 */
void CncAlarmTypeBase::registerEventFields()
{
    // Register event type
    OpcUa_Int16 nsSuperTypeIdx = NodeManagerRoot::getTypeNamespace();
    OpcUa_Int16 nsTypeIdx = NodeManagerCNC::getTypeNamespace();
    EventManagerBase::registerEventType(UaNodeId(OpcUaId_DiscreteAlarmType, nsSuperTypeIdx), UaNodeId(OpcUaCncId_CncAlarmType, nsTypeIdx));
    // Register event fields
    s_CncAlarmTypeDataFields.clear();
    /* AlarmIdentifier*/
    s_CncAlarmTypeDataFields[EventManagerBase::registerEventField(UaQualifiedName("AlarmIdentifier", nsTypeIdx).toFullString())] = 1;
    /* AuxParameters*/
    s_CncAlarmTypeDataFields[EventManagerBase::registerEventField(UaQualifiedName("AuxParameters", nsTypeIdx).toFullString())] = 2;
    /* HelpSource*/
    s_CncAlarmTypeDataFields[EventManagerBase::registerEventField(UaQualifiedName("HelpSource", nsTypeIdx).toFullString())] = 3;
}

/** Registers optional event fields with the EventManagerBase.
 */
OpcUa_UInt32 CncAlarmTypeBase::registerOptionalEventFields(const UaString& fieldName)
{
    OpcUa_UInt32 ret = 0;
    OpcUa_UInt32 externalIndex = EventManagerBase::registerEventField(fieldName);
    std::map<OpcUa_UInt32, OpcUa_UInt32>::iterator it;

    it = s_CncAlarmTypeDataFields.find(externalIndex);
    if ( it != s_CncAlarmTypeDataFields.end() )
    {
        return it->second;
    }

    for ( it=s_CncAlarmTypeDataFields.begin(); it!=s_CncAlarmTypeDataFields.end(); it++ )
    {
        if ( ret < it->second )
        {
            ret = it->second;
        }
    }
    ret++;

    s_CncAlarmTypeDataFields[externalIndex] = ret;

    return ret;
}

/** Get the field value for the passed index.
 *  @param index The index of the selected field.
 *  @param pSession The session context for the method call.
 *  @param data The data for the selected field.
 */
void CncAlarmTypeBase::getFieldData(OpcUa_UInt32 index, Session* pSession, OpcUa_Variant& data)
{
    // Try to find the field index
    std::map<OpcUa_UInt32, OpcUa_UInt32>::iterator it;
    it = s_CncAlarmTypeDataFields.find(index);

    if ( it == s_CncAlarmTypeDataFields.end()  )
    {
        DiscreteAlarmTypeBase::getFieldData(index, pSession, data);
        return;
    }

    switch (it->second)
    {
    case 1: // AlarmIdentifier
        if ( m_FieldValues[ 0 ] == NULL )
        {
            if ( m_pAlarmIdentifier == NULL )
            {
                m_FieldValues[ 0 ] = new UaVariant;
            }
            else
            {
                m_FieldValues[ 0 ] = new UaVariant;
                getAlarmIdentifierValue(pSession, *m_FieldValues[ 0 ]);
            }
        }
        m_FieldValues[ 0 ]->copyTo(&data);
        break;
    case 2: // AuxParameters
        if ( m_FieldValues[ 1 ] == NULL )
        {
            if ( m_pAuxParameters == NULL )
            {
                m_FieldValues[ 1 ] = new UaVariant;
            }
            else
            {
                m_FieldValues[ 1 ] = new UaVariant;
                getAuxParametersValue(pSession, *m_FieldValues[ 1 ]);
            }
        }
        m_FieldValues[ 1 ]->copyTo(&data);
        break;
    case 3: // HelpSource
        if ( m_FieldValues[ 2 ] == NULL )
        {
            if ( m_pHelpSource == NULL )
            {
                m_FieldValues[ 2 ] = new UaVariant;
            }
            else
            {
                m_FieldValues[ 2 ] = new UaVariant;
                getHelpSourceValue(pSession, *m_FieldValues[ 2 ]);
            }
        }
        m_FieldValues[ 2 ]->copyTo(&data);
        break;
    default:
        getCncAlarmTypeOptionalFieldData(it->second, pSession, data);
    }
}

/** Clears the event field data after event processing
*/
void CncAlarmTypeBase::clearFieldData()
{
    m_FieldValues.clear();
    m_FieldValues.create(3);

    DiscreteAlarmTypeBase::clearFieldData();
}

/** Creates a new branch from the current state and adds the branch to a list managed by the condition object.
 *  ConditionType::getBranch() and ConditionType::deleteBranch() can be used to access or to delete the branch.
*/
UaNodeId CncAlarmTypeBase::createBranch()
{
    UaNodeId ret;
    CncAlarmTypeData* pBranch = new CncAlarmTypeData;
    pBranch->initializeAsBranch(this);
    addBranch(pBranch, pBranch->getBranchId());
    return pBranch->getBranchId();
}

/**
 *  Sets the AlarmIdentifier value
 */
void CncAlarmTypeBase::setAlarmIdentifier(const UaString& AlarmIdentifier)
{
    UaVariant value;
    value.setString(AlarmIdentifier);
    UaDataValue dataValue;
    dataValue.setValue(value, OpcUa_True, OpcUa_True);
    m_pAlarmIdentifier->setValue(NULL, dataValue, OpcUa_False);
}

/**
 *  Returns the value of AlarmIdentifier
 */
UaString CncAlarmTypeBase::getAlarmIdentifier() const
{
    UaVariant defaultValue;
    UaString ret;
    UaDataValue dataValue(m_pAlarmIdentifier->value(NULL));
    defaultValue = *dataValue.value();
    ret = defaultValue.toString();
    return ret;
}

/** Set a status code for the event field AlarmIdentifier
 *
 *  If an event field is not available, a StatusCode must be provided
 *  instead of the value to indicate the error situation.
 */
void CncAlarmTypeBase::setAlarmIdentifierStatus(OpcUa_StatusCode status)
{
    UaDataValue dataValue;
    UaDateTime dtNow(UaDateTime::now());
    dataValue.setStatusCode(status);
    dataValue.setSourceTimestamp(dtNow);
    dataValue.setServerTimestamp(dtNow);
    m_pAlarmIdentifier->setValue(NULL, dataValue, OpcUa_False);
}

/** Sets the value of AlarmIdentifier to argument value.
 */
void CncAlarmTypeBase::getAlarmIdentifierValue(Session* pSession, UaVariant& value)
{
    if ( m_pAlarmIdentifier == NULL )
    {
        value.clear();
    }
    else
    {
        UaDataValue dataValue(m_pAlarmIdentifier->value(pSession));
        if ( OpcUa_IsGood(dataValue.statusCode()) )
        {
            value = *(dataValue.value());
        }
        else
        {
            value.setStatusCode(dataValue.statusCode());
        }
    }
}
/**
 *  Sets the AuxParameters value
 *
 *  If the optional AuxParameters node has not been created, this method adds it to the address space.
 */
void CncAlarmTypeBase::setAuxParameters(const UaString& AuxParameters)
{
    if ( m_pAuxParameters == NULL )
    {
        UaStatus addStatus;
        m_pAuxParameters = new OpcUa::PropertyType(this, s_pAuxParameters, m_pNodeConfig, getSharedMutex());
        addStatus = m_pNodeConfig->addNodeAndReference(this, m_pAuxParameters, OpcUaId_HasProperty);
        UA_ASSERT(addStatus.isGood());
    }
    UaVariant value;
    value.setString(AuxParameters);
    UaDataValue dataValue;
    dataValue.setValue(value, OpcUa_True, OpcUa_True);
    m_pAuxParameters->setValue(NULL, dataValue, OpcUa_False);
}

/**
 *  Returns the value of AuxParameters
 *
 *  If the optional AuxParameters node has not been created, the value
 *  of the InstanceDeclaration node of the ObjectType is returned.
 */
UaString CncAlarmTypeBase::getAuxParameters() const
{
    UaVariant defaultValue;
    UaString ret;
    if ( m_pAuxParameters == NULL )
    {
        UaDataValue dataValue(s_pAuxParameters->value(NULL));
        defaultValue = *dataValue.value();
    }
    else
    {
        UaDataValue dataValue(m_pAuxParameters->value(NULL));
        defaultValue = *dataValue.value();
    }
    ret = defaultValue.toString();
    return ret;
}

/** Set a status code for the event field AuxParameters
 *
 *  If an event field is not available, a StatusCode must be provided
 *  instead of the value to indicate the error situation.
 */
void CncAlarmTypeBase::setAuxParametersStatus(OpcUa_StatusCode status)
{
    if ( m_pAuxParameters == NULL )
    {
        UaStatus addStatus;
        m_pAuxParameters = new OpcUa::PropertyType(this, s_pAuxParameters, m_pNodeConfig, getSharedMutex());
        addStatus = m_pNodeConfig->addNodeAndReference(this, m_pAuxParameters, OpcUaId_HasProperty);
        UA_ASSERT(addStatus.isGood());
    }
    UaDataValue dataValue;
    UaDateTime dtNow(UaDateTime::now());
    dataValue.setStatusCode(status);
    dataValue.setSourceTimestamp(dtNow);
    dataValue.setServerTimestamp(dtNow);
    m_pAuxParameters->setValue(NULL, dataValue, OpcUa_False);
}

/** Sets the value of AuxParameters to argument value.
 */
void CncAlarmTypeBase::getAuxParametersValue(Session* pSession, UaVariant& value)
{
    if ( m_pAuxParameters == NULL )
    {
        value.clear();
    }
    else
    {
        UaDataValue dataValue(m_pAuxParameters->value(pSession));
        if ( OpcUa_IsGood(dataValue.statusCode()) )
        {
            value = *(dataValue.value());
        }
        else
        {
            value.setStatusCode(dataValue.statusCode());
        }
    }
}
/**
 *  Sets the HelpSource value
 *
 *  If the optional HelpSource node has not been created, this method adds it to the address space.
 */
void CncAlarmTypeBase::setHelpSource(const UaString& HelpSource)
{
    if ( m_pHelpSource == NULL )
    {
        UaStatus addStatus;
        m_pHelpSource = new OpcUa::PropertyType(this, s_pHelpSource, m_pNodeConfig, getSharedMutex());
        addStatus = m_pNodeConfig->addNodeAndReference(this, m_pHelpSource, OpcUaId_HasProperty);
        UA_ASSERT(addStatus.isGood());
    }
    UaVariant value;
    value.setString(HelpSource);
    UaDataValue dataValue;
    dataValue.setValue(value, OpcUa_True, OpcUa_True);
    m_pHelpSource->setValue(NULL, dataValue, OpcUa_False);
}

/**
 *  Returns the value of HelpSource
 *
 *  If the optional HelpSource node has not been created, the value
 *  of the InstanceDeclaration node of the ObjectType is returned.
 */
UaString CncAlarmTypeBase::getHelpSource() const
{
    UaVariant defaultValue;
    UaString ret;
    if ( m_pHelpSource == NULL )
    {
        UaDataValue dataValue(s_pHelpSource->value(NULL));
        defaultValue = *dataValue.value();
    }
    else
    {
        UaDataValue dataValue(m_pHelpSource->value(NULL));
        defaultValue = *dataValue.value();
    }
    ret = defaultValue.toString();
    return ret;
}

/** Set a status code for the event field HelpSource
 *
 *  If an event field is not available, a StatusCode must be provided
 *  instead of the value to indicate the error situation.
 */
void CncAlarmTypeBase::setHelpSourceStatus(OpcUa_StatusCode status)
{
    if ( m_pHelpSource == NULL )
    {
        UaStatus addStatus;
        m_pHelpSource = new OpcUa::PropertyType(this, s_pHelpSource, m_pNodeConfig, getSharedMutex());
        addStatus = m_pNodeConfig->addNodeAndReference(this, m_pHelpSource, OpcUaId_HasProperty);
        UA_ASSERT(addStatus.isGood());
    }
    UaDataValue dataValue;
    UaDateTime dtNow(UaDateTime::now());
    dataValue.setStatusCode(status);
    dataValue.setSourceTimestamp(dtNow);
    dataValue.setServerTimestamp(dtNow);
    m_pHelpSource->setValue(NULL, dataValue, OpcUa_False);
}

/** Sets the value of HelpSource to argument value.
 */
void CncAlarmTypeBase::getHelpSourceValue(Session* pSession, UaVariant& value)
{
    if ( m_pHelpSource == NULL )
    {
        value.clear();
    }
    else
    {
        UaDataValue dataValue(m_pHelpSource->value(pSession));
        if ( OpcUa_IsGood(dataValue.statusCode()) )
        {
            value = *(dataValue.value());
        }
        else
        {
            value.setStatusCode(dataValue.statusCode());
        }
    }
}

/** Returns the AlarmIdentifier node.
 */
OpcUa::PropertyType* CncAlarmTypeBase::getAlarmIdentifierNode()
{
    return m_pAlarmIdentifier;
}

/** Returns the AlarmIdentifier node.
 */
const OpcUa::PropertyType* CncAlarmTypeBase::getAlarmIdentifierNode() const
{
    return m_pAlarmIdentifier;
}

/** Returns the AuxParameters node.
 *
 * If the Variable has not been created yet, this method returns NULL.
 */
OpcUa::PropertyType* CncAlarmTypeBase::getAuxParametersNode()
{
    return m_pAuxParameters;
}

/** Returns the AuxParameters node.
 *
 * If the Variable has not been created yet, this method returns NULL.
 */
const OpcUa::PropertyType* CncAlarmTypeBase::getAuxParametersNode() const
{
    return m_pAuxParameters;
}

/** Returns the HelpSource node.
 *
 * If the Variable has not been created yet, this method returns NULL.
 */
OpcUa::PropertyType* CncAlarmTypeBase::getHelpSourceNode()
{
    return m_pHelpSource;
}

/** Returns the HelpSource node.
 *
 * If the Variable has not been created yet, this method returns NULL.
 */
const OpcUa::PropertyType* CncAlarmTypeBase::getHelpSourceNode() const
{
    return m_pHelpSource;
}


/** Send event with current event field data to the subscribed clients.
* After setting the changed states of the event this method is used to trigger the event notification that
* contains the snapshot of the new state of the condition.
*/
UaStatus CncAlarmTypeBase::triggerEvent(
    const UaDateTime&   time,        /**< [in] Time provides the time the Event occurred. This value is set as close to the event
                                               generator as possible. It often comes from the underlying system or device. */
    const UaDateTime&   receiveTime, /**< [in] ReceiveTime provides the time the OPC UA Server received the Event from the
                                               underlying device or another Server. */
    const UaByteString& userEventId) /**< [in] The server is responsible for generating a unique EventId for every event sent
                                               to the client. The SDK uses this parameter userEventId as part of the EventId but adds a unique
                                               part. The EventId is provided by the client in calls back to the server like in an alarm
                                               acknowledgement. The user specific part can be extracted from the EventId provided by the client
                                               by using EventManagerBase::getUserEventId(). */
{
    UaMutexLocker lock(m_pSharedMutex);
    clearFieldData();

    m_Time.setDateTime(time);
    m_ReceiveTime.setDateTime(receiveTime);

    // Create a unique EventId sent to the client and include the one passed to this function
    UaByteString clientEventId;
    EventManagerUaNode::buildEventId(userEventId, clientEventId);
    m_EventId.setByteString(clientEventId, OpcUa_True);

    return sendEvent(this);
}

/** Get the field value for the passed index for optional fields of the CncAlarmType.
 *  This function must be overwritten by derived classes to provide the optional fields registered
 *  with CncAlarmTypeData::registerOptionalEventFields.
 *  @param index The index of the selected field.
 *  @param pSession The session context for the method call.
 *  @param data The data for the selected field.
 */
void CncAlarmTypeBase::getCncAlarmTypeOptionalFieldData(OpcUa_UInt32, Session*, OpcUa_Variant& data)
{
    OpcUa_Variant_Initialize(&data);
}
/** Applies the NodeAccessInfo set at the CncAlarmTypeBase and its children. */
void CncAlarmTypeBase::useAccessInfoFromType()
{
    OpcUa::DiscreteAlarmType::useAccessInfoFromType();

    // instance node
    OpcUa_Int16 nsTypeIdx = NodeManagerCNC::getTypeNamespace();
    UaNode *pTypeNode = NodeManagerRoot::CreateRootNodeManager()->getNode(UaNodeId(OpcUaCncId_CncAlarmType, nsTypeIdx));
    if (pTypeNode)
    {
        useAccessInfoFrom(pTypeNode);
        pTypeNode->releaseReference();
    }

    // children
    m_pAlarmIdentifier->useAccessInfoFromInstance(s_pAlarmIdentifier);
    if (m_pAuxParameters)
    {
        m_pAuxParameters->useAccessInfoFromInstance(s_pAuxParameters);
    }
    if (m_pHelpSource)
    {
        m_pHelpSource->useAccessInfoFromInstance(s_pHelpSource);
    }
}

/** Uses the NodeAccessInfo of pOther and its children. */
void CncAlarmTypeBase::useAccessInfoFromInstance(CncAlarmTypeBase *pOther)
{
    OpcUa::DiscreteAlarmType::useAccessInfoFromInstance(pOther);

    // children
    m_pAlarmIdentifier->useAccessInfoFromInstance(pOther->m_pAlarmIdentifier);
    if (m_pAuxParameters && pOther->m_pAuxParameters)
    {
        m_pAuxParameters->useAccessInfoFromInstance(pOther->m_pAuxParameters);
    }
    if (m_pHelpSource && pOther->m_pHelpSource)
    {
        m_pHelpSource->useAccessInfoFromInstance(pOther->m_pHelpSource);
    }
}


// #####################################################################################################
// #####################################################################################################

/** Constructs an CncAlarmTypeData object
*/
CncAlarmTypeData::CncAlarmTypeData()
: OpcUa::DiscreteAlarmTypeData()
{
    initialize();
}

/** Initialize the CncAlarmTypeData object
*/
void CncAlarmTypeData::initialize()
{
    OpcUa_Int16 nsTypeIdx = NodeManagerCNC::getTypeNamespace();
    m_EventTypeId.setNodeId(OpcUaCncId_CncAlarmType, nsTypeIdx);

    // Create a unique EventId sent to the client
    UaByteString clientEventId;
    EventManagerUaNode::buildEventId(UaByteString(), clientEventId);
    m_EventId.setByteString(clientEventId, OpcUa_True);

    m_FieldValues.create(3);

    if ( CncAlarmTypeBase::s_typeNodesCreated == false )
    {
        CncAlarmType::createTypes();
    }
}

/** Destroys the CncAlarmTypeData object
*/
CncAlarmTypeData::~CncAlarmTypeData()
{
}

/** This method is used to initializes the CncAlarmTypeData object with event data
 *  from a condition object to create a branch of this condition object. The method creates a unique BranchId.
 *  EventManagerBase::fireEvent() is used send the intial branch state and status updates for the branch to
 *  the client(s). It is necessary to update the timestamps and the EventId for every update sent to the client(s).
 *  @param pCondition The condition object .
 */
void CncAlarmTypeData::initializeAsBranch(CncAlarmTypeBase* pCondition)
{
    OpcUa::DiscreteAlarmTypeData::initializeAsBranch(pCondition);

    if ( m_FieldValues[ 0 ] == NULL )
    {
        m_FieldValues[ 0 ] = new UaVariant;
    }
    if ( pCondition->m_pAlarmIdentifier )
    {
        *(m_FieldValues[ 0 ]) = *(pCondition->m_pAlarmIdentifier->value(NULL).value());
    }
    else
    {
        *(m_FieldValues[ 0 ]) = UaVariant();
    }
    if ( m_FieldValues[ 1 ] == NULL )
    {
        m_FieldValues[ 1 ] = new UaVariant;
    }
    if ( pCondition->m_pAuxParameters )
    {
        *(m_FieldValues[ 1 ]) = *(pCondition->m_pAuxParameters->value(NULL).value());
    }
    else
    {
        *(m_FieldValues[ 1 ]) = UaVariant();
    }
    if ( m_FieldValues[ 2 ] == NULL )
    {
        m_FieldValues[ 2 ] = new UaVariant;
    }
    if ( pCondition->m_pHelpSource )
    {
        *(m_FieldValues[ 2 ]) = *(pCondition->m_pHelpSource->value(NULL).value());
    }
    else
    {
        *(m_FieldValues[ 2 ]) = UaVariant();
    }
}

/** This method is used to initializes the CncAlarmTypeData object with event data
 *  from another condition data object to create a branch of this condition data object. The method creates a unique BranchId.
 *  EventManagerBase::fireEvent() is used to send the intial branch state and status updates for the branch to
 *  the client(s). It is necessary to update the timestamps and the EventId for every update sent to the client(s).
 *  @param pConditionData The condition data object .
 */
void CncAlarmTypeData::initializeAsBranch(CncAlarmTypeData* pConditionData)
{
    OpcUa::DiscreteAlarmTypeData::initializeAsBranch(pConditionData);

    if ( m_FieldValues[ 0 ] == NULL )
    {
        m_FieldValues[ 0 ] = new UaVariant;
    }
    if ( pConditionData->m_FieldValues[ 0 ] )
    {
        *(m_FieldValues[ 0 ]) = *(pConditionData->m_FieldValues[ 0 ]);
    }
    else
    {
        *(m_FieldValues[ 0 ]) = UaVariant();
    }
    if ( m_FieldValues[ 1 ] == NULL )
    {
        m_FieldValues[ 1 ] = new UaVariant;
    }
    if ( pConditionData->m_FieldValues[ 1 ] )
    {
        *(m_FieldValues[ 1 ]) = *(pConditionData->m_FieldValues[ 1 ]);
    }
    else
    {
        *(m_FieldValues[ 1 ]) = UaVariant();
    }
    if ( m_FieldValues[ 2 ] == NULL )
    {
        m_FieldValues[ 2 ] = new UaVariant;
    }
    if ( pConditionData->m_FieldValues[ 2 ] )
    {
        *(m_FieldValues[ 2 ]) = *(pConditionData->m_FieldValues[ 2 ]);
    }
    else
    {
        *(m_FieldValues[ 2 ]) = UaVariant();
    }
}

/** Get the field value for the passed index.
 *  @param index The index of the selected field.
 *  @param pSession The session context for the method call.
 *  @param data The data for the selected field.
 */
void CncAlarmTypeData::getFieldData(OpcUa_UInt32 index, Session* pSession, OpcUa_Variant& data)
{
    // Try to find the field index
    std::map<OpcUa_UInt32, OpcUa_UInt32>::iterator it;
    it = CncAlarmTypeBase::s_CncAlarmTypeDataFields.find(index);

    if ( it == CncAlarmTypeBase::s_CncAlarmTypeDataFields.end()  )
    {
        DiscreteAlarmTypeData::getFieldData(index, pSession, data);
        return;
    }

    switch (it->second)
    {
        case 1: // AlarmIdentifier
        {
            getAlarmIdentifierValue(pSession, data);
            break;
        }
        case 2: // AuxParameters
        {
            getAuxParametersValue(pSession, data);
            break;
        }
        case 3: // HelpSource
        {
            getHelpSourceValue(pSession, data);
            break;
        }
        default:
        {
            OpcUa_Variant_Clear(&data);
        }
    }
}

/** Set the event field value AlarmIdentifier
 */
void CncAlarmTypeData::setAlarmIdentifier(const UaString& AlarmIdentifier)
{
    if ( m_FieldValues[ 0 ] == NULL )
    {
        m_FieldValues[ 0 ] = new UaVariant;
    }

    m_FieldValues[ 0 ]->setString(AlarmIdentifier);
}
/** Set a status code for the event field value AlarmIdentifier
 *
 * If an event field is not available, a StatusCode must be provided
 * instead of the value to indicate the error situation.
 */
void CncAlarmTypeData::setAlarmIdentifierStatus(OpcUa_StatusCode status)
{
    if ( m_FieldValues[ 0 ] == NULL )
    {
        m_FieldValues[ 0 ] = new UaVariant;
    }

    m_FieldValues[ 0 ]->setStatusCode(status);
}

/** Get the event field value AlarmIdentifier
*/
UaString CncAlarmTypeData::getAlarmIdentifier()
{
    if ( m_FieldValues[ 0 ] == NULL )
    {
        return "";
    }
    else
    {
        return m_FieldValues[ 0 ]->toString();
    }
}
/** Get the event field value AlarmIdentifier for delivering the event data.
 *
 * This method can be overwritten in a derived class to return a client session specific value like a localized text matching the
 * LocaleId requested by the client
 */
void CncAlarmTypeData::getAlarmIdentifierValue(Session* pSession, OpcUa_Variant& value)
{
    OpcUa_ReferenceParameter(pSession);

    if ( m_FieldValues[ 0 ] == NULL )
    {
        OpcUa_Variant_Clear(&value);
    }
    else
    {
        m_FieldValues[ 0 ]->copyTo(&value);
    }
}
/** Set the event field value AuxParameters
 */
void CncAlarmTypeData::setAuxParameters(const UaString& AuxParameters)
{
    if ( m_FieldValues[ 1 ] == NULL )
    {
        m_FieldValues[ 1 ] = new UaVariant;
    }

    m_FieldValues[ 1 ]->setString(AuxParameters);
}
/** Set a status code for the event field value AuxParameters
 *
 * If an event field is not available, a StatusCode must be provided
 * instead of the value to indicate the error situation.
 */
void CncAlarmTypeData::setAuxParametersStatus(OpcUa_StatusCode status)
{
    if ( m_FieldValues[ 1 ] == NULL )
    {
        m_FieldValues[ 1 ] = new UaVariant;
    }

    m_FieldValues[ 1 ]->setStatusCode(status);
}

/** Get the event field value AuxParameters
*/
UaString CncAlarmTypeData::getAuxParameters()
{
    if ( m_FieldValues[ 1 ] == NULL )
    {
        return "";
    }
    else
    {
        return m_FieldValues[ 1 ]->toString();
    }
}
/** Get the event field value AuxParameters for delivering the event data.
 *
 * This method can be overwritten in a derived class to return a client session specific value like a localized text matching the
 * LocaleId requested by the client
 */
void CncAlarmTypeData::getAuxParametersValue(Session* pSession, OpcUa_Variant& value)
{
    OpcUa_ReferenceParameter(pSession);

    if ( m_FieldValues[ 1 ] == NULL )
    {
        OpcUa_Variant_Clear(&value);
    }
    else
    {
        m_FieldValues[ 1 ]->copyTo(&value);
    }
}
/** Set the event field value HelpSource
 */
void CncAlarmTypeData::setHelpSource(const UaString& HelpSource)
{
    if ( m_FieldValues[ 2 ] == NULL )
    {
        m_FieldValues[ 2 ] = new UaVariant;
    }

    m_FieldValues[ 2 ]->setString(HelpSource);
}
/** Set a status code for the event field value HelpSource
 *
 * If an event field is not available, a StatusCode must be provided
 * instead of the value to indicate the error situation.
 */
void CncAlarmTypeData::setHelpSourceStatus(OpcUa_StatusCode status)
{
    if ( m_FieldValues[ 2 ] == NULL )
    {
        m_FieldValues[ 2 ] = new UaVariant;
    }

    m_FieldValues[ 2 ]->setStatusCode(status);
}

/** Get the event field value HelpSource
*/
UaString CncAlarmTypeData::getHelpSource()
{
    if ( m_FieldValues[ 2 ] == NULL )
    {
        return "";
    }
    else
    {
        return m_FieldValues[ 2 ]->toString();
    }
}
/** Get the event field value HelpSource for delivering the event data.
 *
 * This method can be overwritten in a derived class to return a client session specific value like a localized text matching the
 * LocaleId requested by the client
 */
void CncAlarmTypeData::getHelpSourceValue(Session* pSession, OpcUa_Variant& value)
{
    OpcUa_ReferenceParameter(pSession);

    if ( m_FieldValues[ 2 ] == NULL )
    {
        OpcUa_Variant_Clear(&value);
    }
    else
    {
        m_FieldValues[ 2 ]->copyTo(&value);
    }
}


} // End namespace for the UA information model http://opcfoundation.org/UA/CNC/

