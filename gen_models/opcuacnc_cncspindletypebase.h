/******************************************************************************
** opcuacnc_cncspindletypebase.h
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

#ifndef __OPCUACNC_CNCSPINDLETYPEBASE_H__
#define __OPCUACNC_CNCSPINDLETYPEBASE_H__

#include "opcuacnc_cncdrivetype.h"
#include "opcuacnc_datatypes.h"
#include "basenodes.h"
#include "opcua_analogitemtype.h"
#include "opcua_basedatavariabletype.h"
#include "opcua_dataitemtype.h"
#include "opcua_propertytype.h"
#include "opcuacnc_cncpositiontype.h"
#include "opcuacnc_identifiers.h"
#include "opcuacnc_instancefactory_cnc.h"
#include "uaeuinformation.h"
#include "uarange.h"

// Namespace for the UA information model http://opcfoundation.org/UA/CNC/
namespace OpcUaCnc {

class CncPositionVariableType;

/** Generated base class for a CncSpindleType.
 *
 *  This class contains the generated base code for the object type CncSpindleType
 *  representing an OPC UA ObjectType. This class is used to create the object type and to
 *  create and represent instances of the object type in the server address space.
 *
 *  **Variable members of the CncSpindleType:**
 *
 *  Browse Name         | DataType                | TypeDefinition          | Modelling Rule | See Also
 *  --------------------|-------------------------|-------------------------|----------------|---------------------------------------------------------
 *  ActChuckPowerStatus | Boolean                 | DataItemType            | Mandatory      | \ref getActChuckPowerStatus, \ref setActChuckPowerStatus
 *  ActGear             | UInt32                  | DataItemType            | Mandatory      | \ref getActGear, \ref setActGear
 *  ActOverride         | Double                  | AnalogItemType          | Mandatory      | \ref getActOverride, \ref setActOverride
 *  ActSpeed            | Double                  | AnalogItemType          | Mandatory      | \ref getActSpeed, \ref setActSpeed
 *  ActStatus           | CncSpindleStatus        | DataItemType            | Mandatory      | \ref getActStatus, \ref setActStatus
 *  ActTurnDirection    | CncSpindleTurnDirection | DataItemType            | Mandatory      | \ref getActTurnDirection, \ref setActTurnDirection
 *  AnglePos            | CncPositionType         | CncPositionVariableType | Mandatory      | \ref getAnglePos, \ref setAnglePos
 *  CmdGear             | UInt32                  | DataItemType            | Mandatory      | \ref getCmdGear, \ref setCmdGear
 *  CmdOverride         | Double                  | AnalogItemType          | Mandatory      | \ref getCmdOverride, \ref setCmdOverride
 *  CmdSpeed            | Double                  | AnalogItemType          | Mandatory      | \ref getCmdSpeed, \ref setCmdSpeed
 *
 */
class UAMODELS_EXPORT CncSpindleTypeBase:
    public OpcUaCnc::CncDriveType
{
    UA_DISABLE_COPY(CncSpindleTypeBase);
protected:
    virtual ~CncSpindleTypeBase();
public:
    // construction / destruction
    CncSpindleTypeBase(const UaNodeId& nodeId, UaObject* pInstanceDeclarationObject, NodeManagerConfig* pNodeConfig, UaMutexRefCounted* pSharedMutex = NULL);
    CncSpindleTypeBase(const UaNodeId& nodeId, const UaString& name, OpcUa_UInt16 browseNameNameSpaceIndex, NodeManagerConfig* pNodeConfig, UaMutexRefCounted* pSharedMutex = NULL);
    CncSpindleTypeBase(
        UaBase::Object*    pBaseNode,
        XmlUaNodeFactoryManager*   pFactory,
        NodeManagerConfig* pNodeConfig,
        UaMutexRefCounted* pSharedMutex = NULL);
    static void createTypes();
    static void clearStaticMembers();

    virtual UaNodeId       typeDefinitionId() const;


    virtual void setActChuckPowerStatus(OpcUa_Boolean ActChuckPowerStatus);
    virtual OpcUa_Boolean getActChuckPowerStatus() const;

    virtual void setActGear(OpcUa_UInt32 ActGear);
    virtual OpcUa_UInt32 getActGear() const;

    virtual void setActOverride(OpcUa_Double ActOverride);
    virtual OpcUa_Double getActOverride() const;

    virtual void setActSpeed(OpcUa_Double ActSpeed);
    virtual OpcUa_Double getActSpeed() const;

    virtual void setActStatus(OpcUaCnc::CncSpindleStatus ActStatus);
    virtual OpcUaCnc::CncSpindleStatus getActStatus() const;

    virtual void setActTurnDirection(OpcUaCnc::CncSpindleTurnDirection ActTurnDirection);
    virtual OpcUaCnc::CncSpindleTurnDirection getActTurnDirection() const;

    virtual void setAnglePos(const OpcUaCnc::CncPositionType& AnglePos);
    virtual OpcUaCnc::CncPositionType getAnglePos() const;

    virtual void setCmdGear(OpcUa_UInt32 CmdGear);
    virtual OpcUa_UInt32 getCmdGear() const;

    virtual void setCmdOverride(OpcUa_Double CmdOverride);
    virtual OpcUa_Double getCmdOverride() const;

    virtual void setCmdSpeed(OpcUa_Double CmdSpeed);
    virtual OpcUa_Double getCmdSpeed() const;

    virtual OpcUa::DataItemType* getActChuckPowerStatusNode();
    virtual const OpcUa::DataItemType* getActChuckPowerStatusNode() const;
    virtual OpcUa::DataItemType* getActGearNode();
    virtual const OpcUa::DataItemType* getActGearNode() const;
    virtual OpcUa::AnalogItemType* getActOverrideNode();
    virtual const OpcUa::AnalogItemType* getActOverrideNode() const;
    virtual OpcUa::AnalogItemType* getActSpeedNode();
    virtual const OpcUa::AnalogItemType* getActSpeedNode() const;
    virtual OpcUa::DataItemType* getActStatusNode();
    virtual const OpcUa::DataItemType* getActStatusNode() const;
    virtual OpcUa::DataItemType* getActTurnDirectionNode();
    virtual const OpcUa::DataItemType* getActTurnDirectionNode() const;
    virtual OpcUaCnc::CncPositionVariableType* getAnglePosNode();
    virtual const OpcUaCnc::CncPositionVariableType* getAnglePosNode() const;
    virtual OpcUa::DataItemType* getCmdGearNode();
    virtual const OpcUa::DataItemType* getCmdGearNode() const;
    virtual OpcUa::AnalogItemType* getCmdOverrideNode();
    virtual const OpcUa::AnalogItemType* getCmdOverrideNode() const;
    virtual OpcUa::AnalogItemType* getCmdSpeedNode();
    virtual const OpcUa::AnalogItemType* getCmdSpeedNode() const;

    // NodeAccessInfo management
    virtual void useAccessInfoFromType();
    void useAccessInfoFromInstance(CncSpindleTypeBase *pInstance);

protected:
    // Variable nodes
    // Variable ActChuckPowerStatus
    static OpcUa::DataItemType*  s_pActChuckPowerStatus;
    OpcUa::DataItemType*  m_pActChuckPowerStatus;
    // Variable ActGear
    static OpcUa::DataItemType*  s_pActGear;
    OpcUa::DataItemType*  m_pActGear;
    // Variable ActOverride
    static OpcUa::AnalogItemType*  s_pActOverride;
    OpcUa::AnalogItemType*  m_pActOverride;
    // Variable ActSpeed
    static OpcUa::AnalogItemType*  s_pActSpeed;
    OpcUa::AnalogItemType*  m_pActSpeed;
    // Variable ActStatus
    static OpcUa::DataItemType*  s_pActStatus;
    OpcUa::DataItemType*  m_pActStatus;
    // Variable ActTurnDirection
    static OpcUa::DataItemType*  s_pActTurnDirection;
    OpcUa::DataItemType*  m_pActTurnDirection;
    // Variable AnglePos
    static OpcUaCnc::CncPositionVariableType*  s_pAnglePos;
    OpcUaCnc::CncPositionVariableType*  m_pAnglePos;
    // Variable CmdGear
    static OpcUa::DataItemType*  s_pCmdGear;
    OpcUa::DataItemType*  m_pCmdGear;
    // Variable CmdOverride
    static OpcUa::AnalogItemType*  s_pCmdOverride;
    OpcUa::AnalogItemType*  m_pCmdOverride;
    // Variable CmdSpeed
    static OpcUa::AnalogItemType*  s_pCmdSpeed;
    OpcUa::AnalogItemType*  m_pCmdSpeed;



private:
    void initialize();

private:
    static bool s_typeNodesCreated;
};

} // End namespace for the UA information model http://opcfoundation.org/UA/CNC/

#endif // #ifndef __OPCUACNCCNCSPINDLETYPEBASE_H__

