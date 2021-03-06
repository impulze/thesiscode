/******************************************************************************
** thesiscnc_nodemanagertcncbase.cpp
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
** Project: C++ OPC Server SDK information model for namespace http://cs.hs-rm.de/mierswa_thesis/CNC/
**
** Description: OPC Unified Architecture Software Development Kit.
**
** Created: 19.08.2016
**
******************************************************************************/

#include "thesiscnc_nodemanagertcncbase.h"

// Include type headers
#include <uabasenodes.h>
#include "instancefactory.h"
#include "nodemanagerroot.h"
#include "opcua_foldertype.h"

// Namespace for the UA information model http://cs.hs-rm.de/mierswa_thesis/CNC/
namespace ThesisCNC {
OpcUa_UInt16 NodeManagerTCNCBase::s_namespaceIndex = 0;

/** Construction of the class NodeManagerTCNCBase.
*/
NodeManagerTCNCBase::NodeManagerTCNCBase(
    OpcUa_Boolean firesEvents,  //!< [in] Flag indicating if the NodeManager should activate the EventManager
    OpcUa_Int32 nHashTableSize) //!< [in] Size of the hash table. The default value is 10.007.<br>
                                //   The size can be smaller than the total number of nodes managed
                                //   by the node manager but it is better for the performance to avoid
                                //   collisions with a large size.
                                //   Prefered sizes are 1.009, 10.007, 100.003, 1.000.003, 10.000.019.
: NodeManagerBase("http://cs.hs-rm.de/mierswa_thesis/CNC/", firesEvents, nHashTableSize)
{
    m_defaultLocaleId         = "en";
}

/** Finish start up in derived class after starting up base class.
*/
UaStatus NodeManagerTCNCBase::afterStartUp()
{
    UaStatus ret;

    s_namespaceIndex = getNameSpaceIndex();

    ret = createDataTypeNodes();
    UA_ASSERT(ret.isGood());
    ret = createReferenceTypeNodes();
    UA_ASSERT(ret.isGood());

    // ------------------------------------------------------------
    // Create object type nodes
    // ------------------------------------------------------------
    // ------------------------------------------------------------

    // ------------------------------------------------------------
    // Create variable type nodes
    // ------------------------------------------------------------
    // ------------------------------------------------------------

    ret = addAdditionalHierarchicalReferences();

    return ret;
}

/** Static method to get the namespace index for the type namespace managed by this class.
 */
OpcUa_UInt16 NodeManagerTCNCBase::getTypeNamespace()
{
    return s_namespaceIndex;
}

/** Helper function to create data type related nodes.
*/
UaStatus NodeManagerTCNCBase::createDataTypeNodes()
{
    UaStatus               ret;
    // Enumerations


    return ret;
}

/** Helper function to create reference type related nodes.
*/
UaStatus NodeManagerTCNCBase::createReferenceTypeNodes()
{
    UaStatus                ret;

    return ret;
}


UaStatus NodeManagerTCNCBase::addAdditionalHierarchicalReferences()
{
    UaStatus ret;

    return ret;
}


} // End namespace for the UA information model http://cs.hs-rm.de/mierswa_thesis/CNC/



