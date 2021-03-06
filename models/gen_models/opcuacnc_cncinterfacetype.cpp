/******************************************************************************
** opcuacnc_cncinterfacetype.cpp
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

#include "opcuacnc_cncinterfacetype.h"

// Namespace for the UA information model http://opcfoundation.org/UA/CNC/
namespace OpcUaCnc {

/** Construction of the class CncInterfaceType.
 This constructor is used if the object is created based on an instance declaration of an object type. Since the only difference between the
 instance declaration object and the instance object are the node id, the pointer to the instance declaration node is used to get the attribute
 values other than NodeId from the instance declaration node.

 The object is reference counted and can not be deleted directly. The reference counter is set to one after the object is created.
 If the object was successfully added to a NodeManager using the methods NodeManagerConfig::addUaNode or NodeManagerConfig::addNodeAndReference the
 NodeManager will release this reference during shut down. If the object was not added to a NodeManager, it must be deleted using the releaseReference
 method. If the interface pointer is used in other places, the availability of the object must be ensured by incrementing the reference counter with
 addReference when the pointer is stored somewhere and decremented with releaseReference if the interface pointer is not longer needed.
 */
CncInterfaceType::CncInterfaceType(
    const UaNodeId& nodeId,          //!< [in] NodeId of the new object
    UaObject* pInstanceDeclarationObject, //!< [in] UaObject interface of the instance declaration node used to provide attribute values other than NodeId
    NodeManagerConfig* pNodeConfig,  //!< [in] Interface pointer to the NodeManagerConfig interface used to add and delete node and references in the address space
    UaMutexRefCounted* pSharedMutex) //!< [in] Shared mutex object used to synchronize access to the object. Can be NULL if no shared mutex is provided
: CncInterfaceTypeBase(nodeId, pInstanceDeclarationObject, pNodeConfig, pSharedMutex)
{
}

/** Construction of the class CncInterfaceType.
 This constructor is used if the object is not created based on an instance declaration. It defines the name and node id of the object. Additional language specific
 names and other attribute values can be set with set methods provided by the class.

 The object is reference counted and can not be deleted directly. The reference counter is set to one after the object is created.
 If the object was successfully added to a NodeManager using the methods NodeManagerConfig::addUaNode or NodeManagerConfig::addNodeAndReference the
 NodeManager will release this reference during shut down. If the object was not added to a NodeManager, it must be deleted using the releaseReference
 method. If the interface pointer is used in other places, the availability of the object must be ensured by incrementing the reference counter with
 addReference when the pointer is stored somewhere and decremented with releaseReference if the interface pointer is not longer needed.
 */
CncInterfaceType::CncInterfaceType(
    const UaNodeId& nodeId,          //!< [in] NodeId of the new object
    const UaString& name,            //!< [in] Name of the new object. Used as browse name and also as display name if no additional language specific names are set.
    OpcUa_UInt16 browseNameNameSpaceIndex, //!< [in] Namespace index used for the browse name
    NodeManagerConfig* pNodeConfig,  //!< [in] Interface pointer to the NodeManagerConfig interface used to add and delete node and references in the address space
    UaMutexRefCounted* pSharedMutex) //!< [in] Shared mutex object used to synchronize access to the object. Can be NULL if no shared mutex is provided
: CncInterfaceTypeBase(nodeId, name, browseNameNameSpaceIndex, pNodeConfig, pSharedMutex)
{
}

/**  Constructs an instance of the class CncInterfaceType with all components.
*/
CncInterfaceType::CncInterfaceType(
    UaBase::Object*    pBaseNode,    //!< [in] The "tree" of nodes that specifies the attributes of the instance and its children
    XmlUaNodeFactoryManager*   pFactory,     //!< [in] The factory to create the children
    NodeManagerConfig* pNodeConfig,  //!< [in] Interface pointer to the NodeManagerConfig interface used to add and delete node and references in the address space
    UaMutexRefCounted* pSharedMutex) //!< [in] Shared mutex object used to synchronize access to the variable. Can be NULL if no shared mutex is provided
: CncInterfaceTypeBase(pBaseNode, pFactory, pNodeConfig, pSharedMutex)
{
}

/** Destruction
*/
CncInterfaceType::~CncInterfaceType()
{
}

/** Create the type related static members of the class
*/
void CncInterfaceType::createTypes()
{
    // Call base class
    CncInterfaceTypeBase::createTypes();

    // Create your static members here
    // This method may be called several times
}

/** Clear the static members of the class
*/
void CncInterfaceType::clearStaticMembers()
{
    // Clear your static members here
    // Call base class
    CncInterfaceTypeBase::clearStaticMembers();
}

} // End namespace for the UA information model http://opcfoundation.org/UA/CNC/

