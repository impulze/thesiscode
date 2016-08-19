/******************************************************************************
** opcuacnc_nodemanagercnc.h
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

#ifndef _OPCUACNC_NODEMANAGERCNC_H__
#define _OPCUACNC_NODEMANAGERCNC_H__

#include "opcuacnc_nodemanagercncbase.h"

/** Namespace for the UA information model http://opcfoundation.org/UA/CNC/
*/
namespace OpcUaCnc {

/** NodeManager for CNC information model.
 *
 *  This class is used to add custom implementation.
 *  The base class NodeManagerCNCBase contains the auto generated code for creating the adress space.
 */
class UAMODELS_EXPORT NodeManagerCNC : public NodeManagerCNCBase
{
    UA_DISABLE_COPY(NodeManagerCNC);
    NodeManagerCNC();
public:
    /* construction / destruction */
    NodeManagerCNC(OpcUa_Boolean firesEvents, OpcUa_Int32 nHashTableSize = 10007);
    virtual ~NodeManagerCNC();

    //- Interface NodeManagerUaNode -------------------------------------------------------
    virtual UaStatus   afterStartUp();
    virtual UaStatus   beforeShutDown();
    //- Interface NodeManagerUaNode -------------------------------------------------------

};

} // End namespace for the UA information model http://opcfoundation.org/UA/CNC/

#endif // #ifndef __NODEMANAGERCNC_H__

