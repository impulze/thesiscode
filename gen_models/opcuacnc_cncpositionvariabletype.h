/******************************************************************************
** opcuacnc_cncpositionvariabletype.h
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

#ifndef __OPCUACNC_CNCPOSITIONVARIABLETYPE_H__
#define __OPCUACNC_CNCPOSITIONVARIABLETYPE_H__

#include "opcua_basedatavariabletype.h"
#include "opcuacnc_identifiers.h"
#include "opcuacnc_datatypes.h"
#include "opcua_basedatavariabletype.h"
#include "opcua_propertytype.h"
#include "uaeuinformation.h"
#include "uarange.h"

// Namespace for the UA information model http://opcfoundation.org/UA/CNC/
namespace OpcUaCnc {


/** Implements OPC UA Variables of the type CncPositionVariableType
 *
 *  **Variable members of the CncPositionVariableType:**
 *
 *  Browse Name      | DataType      | TypeDefinition       | Modelling Rule | See Also
 *  -----------------|---------------|----------------------|----------------|---------------------------------------------------
 *  ActPos           | Double        | BaseDataVariableType | Mandatory      | \ref getActPos, \ref setActPos
 *  CmdPos           | Double        | BaseDataVariableType | Mandatory      | \ref getCmdPos, \ref setCmdPos
 *  EngineeringUnits | EUInformation | PropertyType         | Mandatory      | \ref getEngineeringUnits, \ref setEngineeringUnits
 *  EURange          | Range         | PropertyType         | Mandatory      | \ref getEURange, \ref setEURange
 *  RemDist          | Double        | BaseDataVariableType | Mandatory      | \ref getRemDist, \ref setRemDist
 *
 */
class UAMODELS_EXPORT CncPositionVariableType:
    public OpcUa::BaseDataVariableType
{
    UA_DISABLE_COPY(CncPositionVariableType);
protected:
    virtual ~CncPositionVariableType();
public:
    CncPositionVariableType(
        UaNode*            pParentNode,
        UaVariable*        pInstanceDeclarationVariable,
        NodeManagerConfig* pNodeConfig,
        UaMutexRefCounted* pSharedMutex = NULL);
    CncPositionVariableType(
        const UaNodeId&    nodeId,
        const UaString&    name,
        OpcUa_UInt16       browseNameNameSpaceIndex,
        const UaVariant&   initialValue,
        OpcUa_Byte         accessLevel,
        NodeManagerConfig* pNodeConfig,
        UaMutexRefCounted* pSharedMutex = NULL);
    CncPositionVariableType(
        UaBase::Variable*  pBaseNode,
        XmlUaNodeFactoryManager*   pFactory,
        NodeManagerConfig* pNodeConfig,
        UaMutexRefCounted* pSharedMutex = NULL);

    static void createTypes();
    static void clearStaticMembers();

    virtual UaNodeId       typeDefinitionId() const;

    virtual void setActPos(OpcUa_Double ActPos);
    virtual OpcUa_Double getActPos() const;

    virtual void setCmdPos(OpcUa_Double CmdPos);
    virtual OpcUa_Double getCmdPos() const;

    virtual void setEngineeringUnits(const UaEUInformation& EngineeringUnits);
    virtual UaEUInformation getEngineeringUnits() const;

    virtual void setEURange(const UaRange& EURange);
    virtual UaRange getEURange() const;

    virtual void setRemDist(OpcUa_Double RemDist);
    virtual OpcUa_Double getRemDist() const;

    virtual OpcUa::BaseDataVariableType* getActPosNode();
    virtual const OpcUa::BaseDataVariableType* getActPosNode() const;
    virtual OpcUa::BaseDataVariableType* getCmdPosNode();
    virtual const OpcUa::BaseDataVariableType* getCmdPosNode() const;
    virtual OpcUa::PropertyType* getEngineeringUnitsNode();
    virtual const OpcUa::PropertyType* getEngineeringUnitsNode() const;
    virtual OpcUa::PropertyType* getEURangeNode();
    virtual const OpcUa::PropertyType* getEURangeNode() const;
    virtual OpcUa::BaseDataVariableType* getRemDistNode();
    virtual const OpcUa::BaseDataVariableType* getRemDistNode() const;

    // NodeAccessInfo management
    virtual void useAccessInfoFromType();
    void useAccessInfoFromInstance(CncPositionVariableType *pInstance);

protected:
    // Variable nodes
    // Variable ActPos
    static OpcUa::BaseDataVariableType*  s_pActPos;
    OpcUa::BaseDataVariableType*  m_pActPos;
    // Variable CmdPos
    static OpcUa::BaseDataVariableType*  s_pCmdPos;
    OpcUa::BaseDataVariableType*  m_pCmdPos;
    // Variable EngineeringUnits
    static OpcUa::PropertyType*  s_pEngineeringUnits;
    OpcUa::PropertyType*  m_pEngineeringUnits;
    // Variable EURange
    static OpcUa::PropertyType*  s_pEURange;
    OpcUa::PropertyType*  m_pEURange;
    // Variable RemDist
    static OpcUa::BaseDataVariableType*  s_pRemDist;
    OpcUa::BaseDataVariableType*  m_pRemDist;


private:
    void initialize(NodeManagerConfig* pNodeConfig);

private:
    static bool s_typeNodesCreated;
};

} // End namespace for the UA information model http://opcfoundation.org/UA/CNC/

#endif // #ifndef __OPCUACNC_CNCPOSITIONVARIABLETYPE_H__

