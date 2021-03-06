/******************************************************************************
** opcuacnc_cncaxistypebase.cpp
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

#include "opcuacnc_cncaxistypebase.h"
#include "opcuacnc_cncaxistype.h"
#include "uagenericnodes.h"
#include "nodemanagerroot.h"
#include "methodhandleuanode.h"
#include "opcuacnc_nodemanagercnc.h"

#include "instancefactory.h"
#include "opcuacnc_cncpositionvariabletype.h"
#include "opcuacnc_instancefactory_cnc.h"
// Namespace for the UA information model http://opcfoundation.org/UA/CNC/
namespace OpcUaCnc {

bool CncAxisTypeBase::s_typeNodesCreated = false;
OpcUa::DataItemType* CncAxisTypeBase::s_pActStatus = NULL;
OpcUa::DataItemType* CncAxisTypeBase::s_pIsReferenced = NULL;
OpcUa::DataItemType* CncAxisTypeBase::s_pIsRotational = NULL;
OpcUaCnc::CncPositionVariableType* CncAxisTypeBase::s_pPosDirect = NULL;
OpcUaCnc::CncPositionVariableType* CncAxisTypeBase::s_pPosIndirect = NULL;
OpcUa::AnalogItemType* CncAxisTypeBase::s_pZeroOffset = NULL;

/** Constructs an CncAxisType object using an instance declaration node as base
*/
CncAxisTypeBase::CncAxisTypeBase(const UaNodeId& nodeId, UaObject* pInstanceDeclarationObject, NodeManagerConfig* pNodeConfig, UaMutexRefCounted* pSharedMutex)
: OpcUaCnc::CncDriveType(nodeId, pInstanceDeclarationObject, pNodeConfig, pSharedMutex)
{
    initialize();
}

/** Constructs an CncAxisType object
*/
CncAxisTypeBase::CncAxisTypeBase(const UaNodeId& nodeId, const UaString& name, OpcUa_UInt16 browseNameNameSpaceIndex, NodeManagerConfig* pNodeConfig, UaMutexRefCounted* pSharedMutex)
: OpcUaCnc::CncDriveType(nodeId, name, browseNameNameSpaceIndex, pNodeConfig, pSharedMutex)
{
    initialize();
}

/**  Constructs an instance of the class CncAxisTypeBase with all components
*/
CncAxisTypeBase::CncAxisTypeBase(
    UaBase::Object*    pBaseNode,    //!< [in] The "tree" of nodes that specifies the attributes of the instance and its children
    XmlUaNodeFactoryManager* pFactory, //!< [in] The factory to create the children
    NodeManagerConfig* pNodeConfig,  //!< [in] Interface pointer to the NodeManagerConfig interface used to add and delete node and references in the address space
    UaMutexRefCounted* pSharedMutex) //!< [in] Shared mutex object used to synchronize access to the variable. Can be NULL if no shared mutex is provided
: OpcUaCnc::CncDriveType(pBaseNode, pFactory, pNodeConfig, pSharedMutex)
{
    UaStatus      addStatus;

    if ( s_typeNodesCreated == false )
    {
        createTypes();
    }

    m_pActStatus = NULL;
    m_pIsReferenced = NULL;
    m_pIsRotational = NULL;
    m_pPosDirect = NULL;
    m_pPosIndirect = NULL;
    m_pZeroOffset = NULL;

    std::list<UaBase::BaseNode*> lstReferencedNodes = pBaseNode->hierarchicallyReferencedNodes();
    for (std::list<UaBase::BaseNode*>::const_iterator it = lstReferencedNodes.begin(); it != lstReferencedNodes.end(); it++)
    {
        UaBase::BaseNode *pChild = *it;
        if (pChild->browseName() == UaQualifiedName("ActStatus", OpcUaCnc::NodeManagerCNC::getTypeNamespace()))
        {
            if (!m_pActStatus)
            {
                m_pActStatus = (OpcUa::DataItemType*) pFactory->createVariable((UaBase::Variable*)pChild, pNodeConfig, pSharedMutex);
                addStatus = pNodeConfig->addNodeAndReference(this, m_pActStatus, OpcUaId_HasComponent);
                UA_ASSERT(addStatus.isGood());
                if (!((UaBase::Variable*)pChild)->modellingRuleId().isNull())
                {
                    m_pActStatus->setModellingRuleId(((UaBase::Variable*)pChild)->modellingRuleId());
                }
            }
        }
        else if (pChild->browseName() == UaQualifiedName("IsReferenced", OpcUaCnc::NodeManagerCNC::getTypeNamespace()))
        {
            if (!m_pIsReferenced)
            {
                m_pIsReferenced = (OpcUa::DataItemType*) pFactory->createVariable((UaBase::Variable*)pChild, pNodeConfig, pSharedMutex);
                addStatus = pNodeConfig->addNodeAndReference(this, m_pIsReferenced, OpcUaId_HasComponent);
                UA_ASSERT(addStatus.isGood());
                if (!((UaBase::Variable*)pChild)->modellingRuleId().isNull())
                {
                    m_pIsReferenced->setModellingRuleId(((UaBase::Variable*)pChild)->modellingRuleId());
                }
            }
        }
        else if (pChild->browseName() == UaQualifiedName("IsRotational", OpcUaCnc::NodeManagerCNC::getTypeNamespace()))
        {
            if (!m_pIsRotational)
            {
                m_pIsRotational = (OpcUa::DataItemType*) pFactory->createVariable((UaBase::Variable*)pChild, pNodeConfig, pSharedMutex);
                addStatus = pNodeConfig->addNodeAndReference(this, m_pIsRotational, OpcUaId_HasComponent);
                UA_ASSERT(addStatus.isGood());
                if (!((UaBase::Variable*)pChild)->modellingRuleId().isNull())
                {
                    m_pIsRotational->setModellingRuleId(((UaBase::Variable*)pChild)->modellingRuleId());
                }
            }
        }
        else if (pChild->browseName() == UaQualifiedName("PosDirect", OpcUaCnc::NodeManagerCNC::getTypeNamespace()))
        {
            if (!m_pPosDirect)
            {
                m_pPosDirect = (OpcUaCnc::CncPositionVariableType*) pFactory->createVariable((UaBase::Variable*)pChild, pNodeConfig, pSharedMutex);
                addStatus = pNodeConfig->addNodeAndReference(this, m_pPosDirect, OpcUaId_HasComponent);
                UA_ASSERT(addStatus.isGood());
                if (!((UaBase::Variable*)pChild)->modellingRuleId().isNull())
                {
                    m_pPosDirect->setModellingRuleId(((UaBase::Variable*)pChild)->modellingRuleId());
                }
            }
        }
        else if (pChild->browseName() == UaQualifiedName("PosIndirect", OpcUaCnc::NodeManagerCNC::getTypeNamespace()))
        {
            if (!m_pPosIndirect)
            {
                m_pPosIndirect = (OpcUaCnc::CncPositionVariableType*) pFactory->createVariable((UaBase::Variable*)pChild, pNodeConfig, pSharedMutex);
                addStatus = pNodeConfig->addNodeAndReference(this, m_pPosIndirect, OpcUaId_HasComponent);
                UA_ASSERT(addStatus.isGood());
                if (!((UaBase::Variable*)pChild)->modellingRuleId().isNull())
                {
                    m_pPosIndirect->setModellingRuleId(((UaBase::Variable*)pChild)->modellingRuleId());
                }
            }
        }
        else if (pChild->browseName() == UaQualifiedName("ZeroOffset", OpcUaCnc::NodeManagerCNC::getTypeNamespace()))
        {
            if (!m_pZeroOffset)
            {
                m_pZeroOffset = (OpcUa::AnalogItemType*) pFactory->createVariable((UaBase::Variable*)pChild, pNodeConfig, pSharedMutex);
                addStatus = pNodeConfig->addNodeAndReference(this, m_pZeroOffset, OpcUaId_HasComponent);
                UA_ASSERT(addStatus.isGood());
                if (!((UaBase::Variable*)pChild)->modellingRuleId().isNull())
                {
                    m_pZeroOffset->setModellingRuleId(((UaBase::Variable*)pChild)->modellingRuleId());
                }
            }
        }
    }
    UA_ASSERT(m_pActStatus);
    UA_ASSERT(m_pIsReferenced);
    UA_ASSERT(m_pIsRotational);
    UA_ASSERT(m_pPosDirect);
    UA_ASSERT(m_pPosIndirect);
    UA_ASSERT(m_pZeroOffset);
}

/** Initialize the object with all member nodes
*/
void CncAxisTypeBase::initialize()
{
    UaStatus      addStatus;
    UaVariant     defaultValue;

    if ( s_typeNodesCreated == false )
    {
        createTypes();
    }
    // Mandatory variable ActStatus
    m_pActStatus = new OpcUa::DataItemType(this, s_pActStatus, m_pNodeConfig, m_pSharedMutex);
    addStatus = m_pNodeConfig->addNodeAndReference(this, m_pActStatus, OpcUaId_HasComponent);
    UA_ASSERT(addStatus.isGood());
    m_pActStatus->setDataType(UaNodeId(OpcUaCncId_CncAxisStatus, OpcUaCnc::NodeManagerCNC::getTypeNamespace()));

    // Mandatory variable IsReferenced
    m_pIsReferenced = new OpcUa::DataItemType(this, s_pIsReferenced, m_pNodeConfig, m_pSharedMutex);
    addStatus = m_pNodeConfig->addNodeAndReference(this, m_pIsReferenced, OpcUaId_HasComponent);
    UA_ASSERT(addStatus.isGood());

    // Mandatory variable IsRotational
    m_pIsRotational = new OpcUa::DataItemType(this, s_pIsRotational, m_pNodeConfig, m_pSharedMutex);
    addStatus = m_pNodeConfig->addNodeAndReference(this, m_pIsRotational, OpcUaId_HasComponent);
    UA_ASSERT(addStatus.isGood());

    // Mandatory variable PosDirect
    m_pPosDirect = new OpcUaCnc::CncPositionVariableType(this, s_pPosDirect, m_pNodeConfig, m_pSharedMutex);
    addStatus = m_pNodeConfig->addNodeAndReference(this, m_pPosDirect, OpcUaId_HasComponent);
    UA_ASSERT(addStatus.isGood());
    m_pPosDirect->setDataType(UaNodeId(OpcUaCncId_CncPositionType, OpcUaCnc::NodeManagerCNC::getTypeNamespace()));

    // Mandatory variable PosIndirect
    m_pPosIndirect = new OpcUaCnc::CncPositionVariableType(this, s_pPosIndirect, m_pNodeConfig, m_pSharedMutex);
    addStatus = m_pNodeConfig->addNodeAndReference(this, m_pPosIndirect, OpcUaId_HasComponent);
    UA_ASSERT(addStatus.isGood());
    m_pPosIndirect->setDataType(UaNodeId(OpcUaCncId_CncPositionType, OpcUaCnc::NodeManagerCNC::getTypeNamespace()));

    // Mandatory variable ZeroOffset
    m_pZeroOffset = new OpcUa::AnalogItemType(this, s_pZeroOffset, m_pNodeConfig, m_pSharedMutex);
    addStatus = m_pNodeConfig->addNodeAndReference(this, m_pZeroOffset, OpcUaId_HasComponent);
    UA_ASSERT(addStatus.isGood());


}

/** Destruction
*/
CncAxisTypeBase::~CncAxisTypeBase()
{
}

/** Create the related type nodes
*/
void CncAxisTypeBase::createTypes()
{
    if ( s_typeNodesCreated == false )
    {
        s_typeNodesCreated = true;

        // Check if supertype is already created
        OpcUaCnc::CncDriveType::createTypes();

        UaStatus      addStatus;
        UaVariant     defaultValue;
        NodeManagerRoot* pNodeManagerRoot = NodeManagerRoot::CreateRootNodeManager();
        OpcUa_Int16 nsTypeIdx = NodeManagerCNC::getTypeNamespace();
        OpcUa_Int16 nsSuperTypeIdx = OpcUaCnc::NodeManagerCNC::getTypeNamespace();
        NodeManagerConfig* pTypeNodeConfig = pNodeManagerRoot->getNodeManagerByNamespace(nsTypeIdx)->getNodeManagerConfig();

        OpcUa::GenericObjectType* pObjectType;
        pObjectType = new OpcUa::GenericObjectType(
            UaNodeId(OpcUaCncId_CncAxisType, nsTypeIdx),
            UaQualifiedName("CncAxisType", nsTypeIdx),
            UaLocalizedText("", "CncAxisType"),
            UaLocalizedText("", ""),
            OpcUa_False,
            &CncAxisType::clearStaticMembers);
        pTypeNodeConfig->addNodeAndReference(UaNodeId(OpcUaCncId_CncDriveType, nsSuperTypeIdx), pObjectType, OpcUaId_HasSubtype);

        // Mandatory variable ActStatus
        defaultValue.setInt32((OpcUa_Int32) (OpcUaCnc::CncAxisStatus)0);
        s_pActStatus = new OpcUa::DataItemType(UaNodeId(OpcUaCncId_CncAxisType_ActStatus, nsTypeIdx), "ActStatus", OpcUaCnc::NodeManagerCNC::getTypeNamespace(), defaultValue, 1, pTypeNodeConfig);
        s_pActStatus->setDataType(UaNodeId(OpcUaCncId_CncAxisStatus, OpcUaCnc::NodeManagerCNC::getTypeNamespace()));
        s_pActStatus->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
        s_pActStatus->setValueRank(-1);
        addStatus = pTypeNodeConfig->addNodeAndReference(pObjectType, s_pActStatus, OpcUaId_HasComponent);
        UA_ASSERT(addStatus.isGood());

        // Mandatory variable IsReferenced
        defaultValue.setBool(OpcUa_False);
        s_pIsReferenced = new OpcUa::DataItemType(UaNodeId(OpcUaCncId_CncAxisType_IsReferenced, nsTypeIdx), "IsReferenced", OpcUaCnc::NodeManagerCNC::getTypeNamespace(), defaultValue, 1, pTypeNodeConfig);
        s_pIsReferenced->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
        s_pIsReferenced->setValueRank(-1);
        addStatus = pTypeNodeConfig->addNodeAndReference(pObjectType, s_pIsReferenced, OpcUaId_HasComponent);
        UA_ASSERT(addStatus.isGood());

        // Mandatory variable IsRotational
        defaultValue.setBool(OpcUa_False);
        s_pIsRotational = new OpcUa::DataItemType(UaNodeId(OpcUaCncId_CncAxisType_IsRotational, nsTypeIdx), "IsRotational", OpcUaCnc::NodeManagerCNC::getTypeNamespace(), defaultValue, 1, pTypeNodeConfig);
        s_pIsRotational->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
        s_pIsRotational->setValueRank(-1);
        addStatus = pTypeNodeConfig->addNodeAndReference(pObjectType, s_pIsRotational, OpcUaId_HasComponent);
        UA_ASSERT(addStatus.isGood());

        // Mandatory variable PosDirect
        OpcUaCnc::CncPositionType valPosDirect;
        valPosDirect.toVariant(defaultValue);
        {
            std::list<UaBase::BaseNode*> lstBaseNodes;
            UaBase::Variable *pChild = new UaBase::Variable(UaNodeId(OpcUaCncId_CncAxisType_PosDirect, nsTypeIdx));
            lstBaseNodes.push_back(pChild);
            pChild->setBrowseName(UaQualifiedName("PosDirect", OpcUaCnc::NodeManagerCNC::getTypeNamespace()));
            pChild->setDisplayName(UaLocalizedText("", "PosDirect"));
            pChild->setAccessLevel(1);
            pChild->setUserAccessLevel(1);
            pChild->setValueRank(-1);
            pChild->setDataTypeId(UaNodeId(OpcUaCncId_CncPositionType, nsTypeIdx));
            pChild->setValue(defaultValue);
            pChild->setValueRank(-1);
            {
                UaBase::BaseNode *pParent = pChild;
                UaBase::BaseNode *pChild2;
                UaBase::FullReference reference;
                pChild2 = new UaBase::Variable(UaNodeId(OpcUaCncId_CncAxisType_PosDirect_ActPos, nsTypeIdx));
                lstBaseNodes.push_back(pChild2);
                pChild2->setBrowseName(UaQualifiedName("ActPos", OpcUaCnc::NodeManagerCNC::getTypeNamespace()));
                pChild2->setDisplayName(UaLocalizedText("", "ActPos"));
                ((UaBase::Variable*)pChild2)->setAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setUserAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setValueRank(-1);
                ((UaBase::Variable*)pChild2)->setDataTypeId(OpcUaId_Double);
                ((UaBase::Variable*)pChild2)->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
                ((UaBase::Variable*)pChild2)->setTypeDefinitionId(OpcUaId_BaseDataVariableType);
                reference.setReferenceTypeId(OpcUaId_HasComponent);
                reference.setSourceNode(pParent);
                reference.setTargetNode(pChild2);
                pParent->addFullReference(reference);
                pChild2->addFullReference(reference);
                pChild2->setParentNodeId(pParent->nodeId());
                pChild2 = new UaBase::Variable(UaNodeId(OpcUaCncId_CncAxisType_PosDirect_CmdPos, nsTypeIdx));
                lstBaseNodes.push_back(pChild2);
                pChild2->setBrowseName(UaQualifiedName("CmdPos", OpcUaCnc::NodeManagerCNC::getTypeNamespace()));
                pChild2->setDisplayName(UaLocalizedText("", "CmdPos"));
                ((UaBase::Variable*)pChild2)->setAccessLevel(3);
                ((UaBase::Variable*)pChild2)->setUserAccessLevel(3);
                ((UaBase::Variable*)pChild2)->setValueRank(-1);
                ((UaBase::Variable*)pChild2)->setDataTypeId(OpcUaId_Double);
                ((UaBase::Variable*)pChild2)->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
                ((UaBase::Variable*)pChild2)->setTypeDefinitionId(OpcUaId_BaseDataVariableType);
                reference.setReferenceTypeId(OpcUaId_HasComponent);
                reference.setSourceNode(pParent);
                reference.setTargetNode(pChild2);
                pParent->addFullReference(reference);
                pChild2->addFullReference(reference);
                pChild2->setParentNodeId(pParent->nodeId());
                pChild2 = new UaBase::Variable(UaNodeId(OpcUaCncId_CncAxisType_PosDirect_EngineeringUnits, nsTypeIdx));
                lstBaseNodes.push_back(pChild2);
                pChild2->setBrowseName(UaQualifiedName("EngineeringUnits", 0));
                pChild2->setDisplayName(UaLocalizedText("", "EngineeringUnits"));
                ((UaBase::Variable*)pChild2)->setAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setUserAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setValueRank(-1);
                ((UaBase::Variable*)pChild2)->setDataTypeId(OpcUaId_EUInformation);
                ((UaBase::Variable*)pChild2)->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
                ((UaBase::Variable*)pChild2)->setTypeDefinitionId(OpcUaId_PropertyType);
                reference.setReferenceTypeId(OpcUaId_HasProperty);
                reference.setSourceNode(pParent);
                reference.setTargetNode(pChild2);
                pParent->addFullReference(reference);
                pChild2->addFullReference(reference);
                pChild2->setParentNodeId(pParent->nodeId());
                pChild2 = new UaBase::Variable(UaNodeId(OpcUaCncId_CncAxisType_PosDirect_EURange, nsTypeIdx));
                lstBaseNodes.push_back(pChild2);
                pChild2->setBrowseName(UaQualifiedName("EURange", 0));
                pChild2->setDisplayName(UaLocalizedText("", "EURange"));
                ((UaBase::Variable*)pChild2)->setAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setUserAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setValueRank(-1);
                ((UaBase::Variable*)pChild2)->setDataTypeId(OpcUaId_Range);
                ((UaBase::Variable*)pChild2)->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
                ((UaBase::Variable*)pChild2)->setTypeDefinitionId(OpcUaId_PropertyType);
                reference.setReferenceTypeId(OpcUaId_HasProperty);
                reference.setSourceNode(pParent);
                reference.setTargetNode(pChild2);
                pParent->addFullReference(reference);
                pChild2->addFullReference(reference);
                pChild2->setParentNodeId(pParent->nodeId());
                pChild2 = new UaBase::Variable(UaNodeId(OpcUaCncId_CncAxisType_PosDirect_RemDist, nsTypeIdx));
                lstBaseNodes.push_back(pChild2);
                pChild2->setBrowseName(UaQualifiedName("RemDist", OpcUaCnc::NodeManagerCNC::getTypeNamespace()));
                pChild2->setDisplayName(UaLocalizedText("", "RemDist"));
                ((UaBase::Variable*)pChild2)->setAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setUserAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setValueRank(-1);
                ((UaBase::Variable*)pChild2)->setDataTypeId(OpcUaId_Double);
                ((UaBase::Variable*)pChild2)->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
                ((UaBase::Variable*)pChild2)->setTypeDefinitionId(OpcUaId_BaseDataVariableType);
                reference.setReferenceTypeId(OpcUaId_HasComponent);
                reference.setSourceNode(pParent);
                reference.setTargetNode(pChild2);
                pParent->addFullReference(reference);
                pChild2->addFullReference(reference);
                pChild2->setParentNodeId(pParent->nodeId());
            }

            XmlUaNodeFactoryManager factory;
            s_pPosDirect = new OpcUaCnc::CncPositionVariableType(pChild, &factory, pTypeNodeConfig, NULL);
            addStatus = pTypeNodeConfig->addNodeAndReference(pObjectType, s_pPosDirect, OpcUaId_HasComponent);
            UA_ASSERT(addStatus.isGood());
            s_pPosDirect->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
            std::list<UaBase::BaseNode*>::iterator it = lstBaseNodes.begin();
            while (it != lstBaseNodes.end())
            {
                delete *it;
                it++;
            }
        }

        // Mandatory variable PosIndirect
        OpcUaCnc::CncPositionType valPosIndirect;
        valPosIndirect.toVariant(defaultValue);
        {
            std::list<UaBase::BaseNode*> lstBaseNodes;
            UaBase::Variable *pChild = new UaBase::Variable(UaNodeId(OpcUaCncId_CncAxisType_PosIndirect, nsTypeIdx));
            lstBaseNodes.push_back(pChild);
            pChild->setBrowseName(UaQualifiedName("PosIndirect", OpcUaCnc::NodeManagerCNC::getTypeNamespace()));
            pChild->setDisplayName(UaLocalizedText("", "PosIndirect"));
            pChild->setAccessLevel(1);
            pChild->setUserAccessLevel(1);
            pChild->setValueRank(-1);
            pChild->setDataTypeId(UaNodeId(OpcUaCncId_CncPositionType, nsTypeIdx));
            pChild->setValue(defaultValue);
            pChild->setValueRank(-1);
            {
                UaBase::BaseNode *pParent = pChild;
                UaBase::BaseNode *pChild2;
                UaBase::FullReference reference;
                pChild2 = new UaBase::Variable(UaNodeId(OpcUaCncId_CncAxisType_PosIndirect_ActPos, nsTypeIdx));
                lstBaseNodes.push_back(pChild2);
                pChild2->setBrowseName(UaQualifiedName("ActPos", OpcUaCnc::NodeManagerCNC::getTypeNamespace()));
                pChild2->setDisplayName(UaLocalizedText("", "ActPos"));
                ((UaBase::Variable*)pChild2)->setAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setUserAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setValueRank(-1);
                ((UaBase::Variable*)pChild2)->setDataTypeId(OpcUaId_Double);
                ((UaBase::Variable*)pChild2)->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
                ((UaBase::Variable*)pChild2)->setTypeDefinitionId(OpcUaId_BaseDataVariableType);
                reference.setReferenceTypeId(OpcUaId_HasComponent);
                reference.setSourceNode(pParent);
                reference.setTargetNode(pChild2);
                pParent->addFullReference(reference);
                pChild2->addFullReference(reference);
                pChild2->setParentNodeId(pParent->nodeId());
                pChild2 = new UaBase::Variable(UaNodeId(OpcUaCncId_CncAxisType_PosIndirect_CmdPos, nsTypeIdx));
                lstBaseNodes.push_back(pChild2);
                pChild2->setBrowseName(UaQualifiedName("CmdPos", OpcUaCnc::NodeManagerCNC::getTypeNamespace()));
                pChild2->setDisplayName(UaLocalizedText("", "CmdPos"));
                ((UaBase::Variable*)pChild2)->setAccessLevel(3);
                ((UaBase::Variable*)pChild2)->setUserAccessLevel(3);
                ((UaBase::Variable*)pChild2)->setValueRank(-1);
                ((UaBase::Variable*)pChild2)->setDataTypeId(OpcUaId_Double);
                ((UaBase::Variable*)pChild2)->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
                ((UaBase::Variable*)pChild2)->setTypeDefinitionId(OpcUaId_BaseDataVariableType);
                reference.setReferenceTypeId(OpcUaId_HasComponent);
                reference.setSourceNode(pParent);
                reference.setTargetNode(pChild2);
                pParent->addFullReference(reference);
                pChild2->addFullReference(reference);
                pChild2->setParentNodeId(pParent->nodeId());
                pChild2 = new UaBase::Variable(UaNodeId(OpcUaCncId_CncAxisType_PosIndirect_EngineeringUnits, nsTypeIdx));
                lstBaseNodes.push_back(pChild2);
                pChild2->setBrowseName(UaQualifiedName("EngineeringUnits", 0));
                pChild2->setDisplayName(UaLocalizedText("", "EngineeringUnits"));
                ((UaBase::Variable*)pChild2)->setAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setUserAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setValueRank(-1);
                ((UaBase::Variable*)pChild2)->setDataTypeId(OpcUaId_EUInformation);
                ((UaBase::Variable*)pChild2)->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
                ((UaBase::Variable*)pChild2)->setTypeDefinitionId(OpcUaId_PropertyType);
                reference.setReferenceTypeId(OpcUaId_HasProperty);
                reference.setSourceNode(pParent);
                reference.setTargetNode(pChild2);
                pParent->addFullReference(reference);
                pChild2->addFullReference(reference);
                pChild2->setParentNodeId(pParent->nodeId());
                pChild2 = new UaBase::Variable(UaNodeId(OpcUaCncId_CncAxisType_PosIndirect_EURange, nsTypeIdx));
                lstBaseNodes.push_back(pChild2);
                pChild2->setBrowseName(UaQualifiedName("EURange", 0));
                pChild2->setDisplayName(UaLocalizedText("", "EURange"));
                ((UaBase::Variable*)pChild2)->setAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setUserAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setValueRank(-1);
                ((UaBase::Variable*)pChild2)->setDataTypeId(OpcUaId_Range);
                ((UaBase::Variable*)pChild2)->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
                ((UaBase::Variable*)pChild2)->setTypeDefinitionId(OpcUaId_PropertyType);
                reference.setReferenceTypeId(OpcUaId_HasProperty);
                reference.setSourceNode(pParent);
                reference.setTargetNode(pChild2);
                pParent->addFullReference(reference);
                pChild2->addFullReference(reference);
                pChild2->setParentNodeId(pParent->nodeId());
                pChild2 = new UaBase::Variable(UaNodeId(OpcUaCncId_CncAxisType_PosIndirect_RemDist, nsTypeIdx));
                lstBaseNodes.push_back(pChild2);
                pChild2->setBrowseName(UaQualifiedName("RemDist", OpcUaCnc::NodeManagerCNC::getTypeNamespace()));
                pChild2->setDisplayName(UaLocalizedText("", "RemDist"));
                ((UaBase::Variable*)pChild2)->setAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setUserAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setValueRank(-1);
                ((UaBase::Variable*)pChild2)->setDataTypeId(OpcUaId_Double);
                ((UaBase::Variable*)pChild2)->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
                ((UaBase::Variable*)pChild2)->setTypeDefinitionId(OpcUaId_BaseDataVariableType);
                reference.setReferenceTypeId(OpcUaId_HasComponent);
                reference.setSourceNode(pParent);
                reference.setTargetNode(pChild2);
                pParent->addFullReference(reference);
                pChild2->addFullReference(reference);
                pChild2->setParentNodeId(pParent->nodeId());
            }

            XmlUaNodeFactoryManager factory;
            s_pPosIndirect = new OpcUaCnc::CncPositionVariableType(pChild, &factory, pTypeNodeConfig, NULL);
            addStatus = pTypeNodeConfig->addNodeAndReference(pObjectType, s_pPosIndirect, OpcUaId_HasComponent);
            UA_ASSERT(addStatus.isGood());
            s_pPosIndirect->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
            std::list<UaBase::BaseNode*>::iterator it = lstBaseNodes.begin();
            while (it != lstBaseNodes.end())
            {
                delete *it;
                it++;
            }
        }

        // Mandatory variable ZeroOffset
        defaultValue.setBool(OpcUa_False);
        {
            std::list<UaBase::BaseNode*> lstBaseNodes;
            UaBase::Variable *pChild = new UaBase::Variable(UaNodeId(OpcUaCncId_CncAxisType_ZeroOffset, nsTypeIdx));
            lstBaseNodes.push_back(pChild);
            pChild->setBrowseName(UaQualifiedName("ZeroOffset", OpcUaCnc::NodeManagerCNC::getTypeNamespace()));
            pChild->setDisplayName(UaLocalizedText("", "ZeroOffset"));
            pChild->setAccessLevel(1);
            pChild->setUserAccessLevel(1);
            pChild->setValueRank(-1);
            pChild->setDataTypeId(OpcUaId_Boolean);
            pChild->setValue(defaultValue);
            pChild->setValueRank(-1);
            {
                UaBase::BaseNode *pParent = pChild;
                UaBase::BaseNode *pChild2;
                UaBase::FullReference reference;
                pChild2 = new UaBase::Variable(UaNodeId(OpcUaCncId_CncAxisType_ZeroOffset_EURange, nsTypeIdx));
                lstBaseNodes.push_back(pChild2);
                pChild2->setBrowseName(UaQualifiedName("EURange", 0));
                pChild2->setDisplayName(UaLocalizedText("", "EURange"));
                ((UaBase::Variable*)pChild2)->setAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setUserAccessLevel(1);
                ((UaBase::Variable*)pChild2)->setValueRank(-1);
                ((UaBase::Variable*)pChild2)->setDataTypeId(OpcUaId_Range);
                ((UaBase::Variable*)pChild2)->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
                ((UaBase::Variable*)pChild2)->setTypeDefinitionId(OpcUaId_PropertyType);
                reference.setReferenceTypeId(OpcUaId_HasProperty);
                reference.setSourceNode(pParent);
                reference.setTargetNode(pChild2);
                pParent->addFullReference(reference);
                pChild2->addFullReference(reference);
                pChild2->setParentNodeId(pParent->nodeId());
            }

            XmlUaNodeFactoryManager factory;
            s_pZeroOffset = new OpcUa::AnalogItemType(pChild, &factory, pTypeNodeConfig, NULL);
            addStatus = pTypeNodeConfig->addNodeAndReference(pObjectType, s_pZeroOffset, OpcUaId_HasComponent);
            UA_ASSERT(addStatus.isGood());
            s_pZeroOffset->setModellingRuleId(OpcUaId_ModellingRule_Mandatory);
            std::list<UaBase::BaseNode*>::iterator it = lstBaseNodes.begin();
            while (it != lstBaseNodes.end())
            {
                delete *it;
                it++;
            }
        }

    }
}

/** Clear the static members of the class
*/
void CncAxisTypeBase::clearStaticMembers()
{
    s_typeNodesCreated = false;

    s_pActStatus = NULL;
    s_pIsReferenced = NULL;
    s_pIsRotational = NULL;
    s_pPosDirect = NULL;
    s_pPosIndirect = NULL;
    s_pZeroOffset = NULL;
}

/** Returns the type definition NodeId for the CncAxisType
*/
UaNodeId CncAxisTypeBase::typeDefinitionId() const
{
    UaNodeId ret(OpcUaCncId_CncAxisType, NodeManagerCNC::getTypeNamespace());
    return ret;
}

/**
 *  Sets the ActStatus value
 */
void CncAxisTypeBase::setActStatus(OpcUaCnc::CncAxisStatus ActStatus)
{
    UaVariant value;
    value.setInt32(ActStatus);
    UaDataValue dataValue;
    dataValue.setValue(value, OpcUa_True, OpcUa_True);
    m_pActStatus->setValue(NULL, dataValue, OpcUa_False);
}

/**
 *  Returns the value of ActStatus
 */
OpcUaCnc::CncAxisStatus CncAxisTypeBase::getActStatus() const
{
    UaVariant defaultValue;
    OpcUaCnc::CncAxisStatus ret = (OpcUaCnc::CncAxisStatus)0;
    UaDataValue dataValue(m_pActStatus->value(NULL));
    defaultValue = *dataValue.value();
    OpcUa_Int32 i = (OpcUa_Int32) (OpcUaCnc::CncAxisStatus)0;
    defaultValue.toInt32(i);
    ret = (OpcUaCnc::CncAxisStatus) i;
    return ret;
}

/**
 *  Sets the IsReferenced value
 */
void CncAxisTypeBase::setIsReferenced(OpcUa_Boolean IsReferenced)
{
    UaVariant value;
    value.setBool(IsReferenced);
    UaDataValue dataValue;
    dataValue.setValue(value, OpcUa_True, OpcUa_True);
    m_pIsReferenced->setValue(NULL, dataValue, OpcUa_False);
}

/**
 *  Returns the value of IsReferenced
 */
OpcUa_Boolean CncAxisTypeBase::getIsReferenced() const
{
    UaVariant defaultValue;
    OpcUa_Boolean ret = OpcUa_False;
    UaDataValue dataValue(m_pIsReferenced->value(NULL));
    defaultValue = *dataValue.value();
    defaultValue.toBool(ret);
    return ret;
}

/**
 *  Sets the IsRotational value
 */
void CncAxisTypeBase::setIsRotational(OpcUa_Boolean IsRotational)
{
    UaVariant value;
    value.setBool(IsRotational);
    UaDataValue dataValue;
    dataValue.setValue(value, OpcUa_True, OpcUa_True);
    m_pIsRotational->setValue(NULL, dataValue, OpcUa_False);
}

/**
 *  Returns the value of IsRotational
 */
OpcUa_Boolean CncAxisTypeBase::getIsRotational() const
{
    UaVariant defaultValue;
    OpcUa_Boolean ret = OpcUa_False;
    UaDataValue dataValue(m_pIsRotational->value(NULL));
    defaultValue = *dataValue.value();
    defaultValue.toBool(ret);
    return ret;
}

/**
 *  Sets the PosDirect value
 */
void CncAxisTypeBase::setPosDirect(const OpcUaCnc::CncPositionType& PosDirect)
{
    UaDataValue dataValue;
    PosDirect.toDataValue(dataValue, OpcUa_True);
    m_pPosDirect->setValue(NULL, dataValue, OpcUa_False);
}

/**
 *  Returns the value of PosDirect
 */
OpcUaCnc::CncPositionType CncAxisTypeBase::getPosDirect() const
{
    OpcUaCnc::CncPositionType ret;
    UaDataValue dataValue(m_pPosDirect->value(NULL));
    UaExtensionObject tmpExtensionObject;
    UaVariant(*dataValue.value()).toExtensionObject(tmpExtensionObject);
    ret.setCncPositionType(tmpExtensionObject, OpcUa_True);
    return ret;
}

/**
 *  Sets the PosIndirect value
 */
void CncAxisTypeBase::setPosIndirect(const OpcUaCnc::CncPositionType& PosIndirect)
{
    UaDataValue dataValue;
    PosIndirect.toDataValue(dataValue, OpcUa_True);
    m_pPosIndirect->setValue(NULL, dataValue, OpcUa_False);
}

/**
 *  Returns the value of PosIndirect
 */
OpcUaCnc::CncPositionType CncAxisTypeBase::getPosIndirect() const
{
    OpcUaCnc::CncPositionType ret;
    UaDataValue dataValue(m_pPosIndirect->value(NULL));
    UaExtensionObject tmpExtensionObject;
    UaVariant(*dataValue.value()).toExtensionObject(tmpExtensionObject);
    ret.setCncPositionType(tmpExtensionObject, OpcUa_True);
    return ret;
}

/**
 *  Sets the ZeroOffset value
 */
void CncAxisTypeBase::setZeroOffset(OpcUa_Boolean ZeroOffset)
{
    UaVariant value;
    value.setBool(ZeroOffset);
    UaDataValue dataValue;
    dataValue.setValue(value, OpcUa_True, OpcUa_True);
    m_pZeroOffset->setValue(NULL, dataValue, OpcUa_False);
}

/**
 *  Returns the value of ZeroOffset
 */
OpcUa_Boolean CncAxisTypeBase::getZeroOffset() const
{
    UaVariant defaultValue;
    OpcUa_Boolean ret = OpcUa_False;
    UaDataValue dataValue(m_pZeroOffset->value(NULL));
    defaultValue = *dataValue.value();
    defaultValue.toBool(ret);
    return ret;
}

/** Returns the ActStatus node.
 */
OpcUa::DataItemType* CncAxisTypeBase::getActStatusNode()
{
    return m_pActStatus;
}

/** Returns the ActStatus node.
 */
const OpcUa::DataItemType* CncAxisTypeBase::getActStatusNode() const
{
    return m_pActStatus;
}

/** Returns the IsReferenced node.
 */
OpcUa::DataItemType* CncAxisTypeBase::getIsReferencedNode()
{
    return m_pIsReferenced;
}

/** Returns the IsReferenced node.
 */
const OpcUa::DataItemType* CncAxisTypeBase::getIsReferencedNode() const
{
    return m_pIsReferenced;
}

/** Returns the IsRotational node.
 */
OpcUa::DataItemType* CncAxisTypeBase::getIsRotationalNode()
{
    return m_pIsRotational;
}

/** Returns the IsRotational node.
 */
const OpcUa::DataItemType* CncAxisTypeBase::getIsRotationalNode() const
{
    return m_pIsRotational;
}

/** Returns the PosDirect node.
 */
OpcUaCnc::CncPositionVariableType* CncAxisTypeBase::getPosDirectNode()
{
    return m_pPosDirect;
}

/** Returns the PosDirect node.
 */
const OpcUaCnc::CncPositionVariableType* CncAxisTypeBase::getPosDirectNode() const
{
    return m_pPosDirect;
}

/** Returns the PosIndirect node.
 */
OpcUaCnc::CncPositionVariableType* CncAxisTypeBase::getPosIndirectNode()
{
    return m_pPosIndirect;
}

/** Returns the PosIndirect node.
 */
const OpcUaCnc::CncPositionVariableType* CncAxisTypeBase::getPosIndirectNode() const
{
    return m_pPosIndirect;
}

/** Returns the ZeroOffset node.
 */
OpcUa::AnalogItemType* CncAxisTypeBase::getZeroOffsetNode()
{
    return m_pZeroOffset;
}

/** Returns the ZeroOffset node.
 */
const OpcUa::AnalogItemType* CncAxisTypeBase::getZeroOffsetNode() const
{
    return m_pZeroOffset;
}


/** Applies the NodeAccessInfo set at the CncAxisTypeBase and its children. */
void CncAxisTypeBase::useAccessInfoFromType()
{
    OpcUaCnc::CncDriveType::useAccessInfoFromType();

    // instance node
    OpcUa_Int16 nsTypeIdx = NodeManagerCNC::getTypeNamespace();
    UaNode *pTypeNode = NodeManagerRoot::CreateRootNodeManager()->getNode(UaNodeId(OpcUaCncId_CncAxisType, nsTypeIdx));
    if (pTypeNode)
    {
        useAccessInfoFrom(pTypeNode);
        pTypeNode->releaseReference();
    }

    // children
    m_pActStatus->useAccessInfoFromInstance(s_pActStatus);
    m_pIsReferenced->useAccessInfoFromInstance(s_pIsReferenced);
    m_pIsRotational->useAccessInfoFromInstance(s_pIsRotational);
    m_pPosDirect->useAccessInfoFromInstance(s_pPosDirect);
    m_pPosIndirect->useAccessInfoFromInstance(s_pPosIndirect);
    m_pZeroOffset->useAccessInfoFromInstance(s_pZeroOffset);
}

/** Uses the NodeAccessInfo of pOther and its children. */
void CncAxisTypeBase::useAccessInfoFromInstance(CncAxisTypeBase *pOther)
{
    OpcUaCnc::CncDriveType::useAccessInfoFromInstance(pOther);

    // children
    m_pActStatus->useAccessInfoFromInstance(pOther->m_pActStatus);
    m_pIsReferenced->useAccessInfoFromInstance(pOther->m_pIsReferenced);
    m_pIsRotational->useAccessInfoFromInstance(pOther->m_pIsRotational);
    m_pPosDirect->useAccessInfoFromInstance(pOther->m_pPosDirect);
    m_pPosIndirect->useAccessInfoFromInstance(pOther->m_pPosIndirect);
    m_pZeroOffset->useAccessInfoFromInstance(pOther->m_pZeroOffset);
}

} // End namespace for the UA information model http://opcfoundation.org/UA/CNC/


