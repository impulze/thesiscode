/******************************************************************************
** opcuacnc_cncaxistype.h
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
** Project: C++ OPC Server SDK information model for namespace http://opcfoundation.org/UA/CNC/
**
** Description: OPC Unified Architecture Software Development Kit.
**
** Created: 19.08.2016
**
******************************************************************************/

#ifndef __OPCUACNC_CNCAXISTYPE_H__
#define __OPCUACNC_CNCAXISTYPE_H__

#include "opcuacnc_cncaxistypebase.h"

// Namespace for the UA information model http://opcfoundation.org/UA/CNC/
namespace OpcUaCnc {

/** @brief Class implementing the UaObject interface for the CncAxisType.
 *
 * OPC UA Objects are used to represent systems, system components, real-world objects and software
 * objects. They have the NodeClass Object. The detailed description of Objects and their attributes
 * can be found in the general description of the Object node class.
 *
 *  **Variable members of the CncAxisType:**
 *
 *  Browse Name  | DataType        | TypeDefinition          | Modelling Rule | See Also
 *  -------------|-----------------|-------------------------|----------------|-----------------------------------------------------------------------------------------------------------------
 *  ActStatus    | CncAxisStatus   | DataItemType            | Mandatory      | \ref CncAxisTypeBase::getActStatus "getActStatus", \ref CncAxisTypeBase::setActStatus "setActStatus"
 *  IsReferenced | Boolean         | DataItemType            | Mandatory      | \ref CncAxisTypeBase::getIsReferenced "getIsReferenced", \ref CncAxisTypeBase::setIsReferenced "setIsReferenced"
 *  IsRotational | Boolean         | DataItemType            | Mandatory      | \ref CncAxisTypeBase::getIsRotational "getIsRotational", \ref CncAxisTypeBase::setIsRotational "setIsRotational"
 *  PosDirect    | CncPositionType | CncPositionVariableType | Mandatory      | \ref CncAxisTypeBase::getPosDirect "getPosDirect", \ref CncAxisTypeBase::setPosDirect "setPosDirect"
 *  PosIndirect  | CncPositionType | CncPositionVariableType | Mandatory      | \ref CncAxisTypeBase::getPosIndirect "getPosIndirect", \ref CncAxisTypeBase::setPosIndirect "setPosIndirect"
 *  ZeroOffset   | Boolean         | AnalogItemType          | Mandatory      | \ref CncAxisTypeBase::getZeroOffset "getZeroOffset", \ref CncAxisTypeBase::setZeroOffset "setZeroOffset"
 *
 */
class UAMODELS_EXPORT CncAxisType:
    public CncAxisTypeBase
{
    UA_DISABLE_COPY(CncAxisType);
protected:
    // destruction
    virtual ~CncAxisType();
public:
    // construction
    CncAxisType(const UaNodeId& nodeId, UaObject* pInstanceDeclarationObject, NodeManagerConfig* pNodeConfig, UaMutexRefCounted* pSharedMutex = NULL);
    CncAxisType(const UaNodeId& nodeId, const UaString& name, OpcUa_UInt16 browseNameNameSpaceIndex, NodeManagerConfig* pNodeConfig, UaMutexRefCounted* pSharedMutex = NULL);
    CncAxisType(
        UaBase::Object*    pBaseNode,
        XmlUaNodeFactoryManager*   pFactory,
        NodeManagerConfig* pNodeConfig,
        UaMutexRefCounted* pSharedMutex = NULL);
    static void createTypes();
    static void clearStaticMembers();


protected:

private:
};

} // End namespace for the UA information model http://opcfoundation.org/UA/CNC/

#endif // #ifndef __OPCUACNCCNCAXISTYPE_H__

