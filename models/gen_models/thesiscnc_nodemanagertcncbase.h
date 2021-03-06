/******************************************************************************
** thesiscnc_nodemanagertcncbase.h
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

#ifndef _THESISCNC_NODEMANAGERTCNCBASE_H__
#define _THESISCNC_NODEMANAGERTCNCBASE_H__

#include "uamutex.h"
#include "uabasenodes.h"
#include "nodemanagerbase.h"
#include "uaobjecttypes.h"
#include "thesiscnc_identifiers.h"

/** Namespace for the UA information model http://cs.hs-rm.de/mierswa_thesis/CNC/
*/
namespace ThesisCNC {

/** Generated base class for NodeManager for TCNC information model.
 *
 *  This abstract class contains the auto generated code for the NodeManagerTCNC.
 *  This class is used to create the nodes defined for the namespace http://cs.hs-rm.de/mierswa_thesis/CNC/.
 *
 */
class UAMODELS_EXPORT NodeManagerTCNCBase : public NodeManagerBase
{
    UA_DISABLE_COPY(NodeManagerTCNCBase);
    NodeManagerTCNCBase();
public:
    /* construction / destruction */
    NodeManagerTCNCBase(OpcUa_Boolean firesEvents, OpcUa_Int32 nHashTableSize = 10007);
    virtual ~NodeManagerTCNCBase(){};

    //- Interface NodeManagerUaNode -------------------------------------------------------
    virtual UaStatus   afterStartUp();
    //- Interface NodeManagerUaNode -------------------------------------------------------

    static OpcUa_UInt16 getTypeNamespace();

private:
    UaStatus createDataTypeNodes();
    UaStatus createReferenceTypeNodes();
    UaStatus addAdditionalHierarchicalReferences();


protected:
    UaMutex             m_mutex;
    static OpcUa_UInt16 s_namespaceIndex;
};

} // End namespace for the UA information model http://cs.hs-rm.de/mierswa_thesis/CNC/

#endif // #ifndef __NODEMANAGERTCNCBASE_H__

