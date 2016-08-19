/******************************************************************************
** opcuacnc_nodemanagercncbase.cpp
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

#include "opcuacnc_nodemanagercncbase.h"

// Include type headers
#include "opcuacnc_datatypes.h"
#include <uabasenodes.h>
#include "opcuacnc_cncalarmtype.h"
#include "opcuacnc_cncaxislisttype.h"
#include "opcuacnc_cncaxistype.h"
#include "opcuacnc_cncchannellisttype.h"
#include "opcuacnc_cncchanneltype.h"
#include "opcuacnc_cnccomponenttype.h"
#include "opcuacnc_cncdrivetype.h"
#include "opcuacnc_cncinterfacetype.h"
#include "opcuacnc_cncmessagetype.h"
#include "opcuacnc_cncpositionvariabletype.h"
#include "opcuacnc_cncspindlelisttype.h"
#include "opcuacnc_cncspindletype.h"
#include "opcua_propertytype.h"
#include "instancefactory.h"
#include "nodemanagerroot.h"
#include "opcua_foldertype.h"

// Namespace for the UA information model http://opcfoundation.org/UA/CNC/
namespace OpcUaCnc {
OpcUa_UInt16 NodeManagerCNCBase::s_namespaceIndex = 0;

/** Construction of the class NodeManagerCNCBase.
*/
NodeManagerCNCBase::NodeManagerCNCBase(
    OpcUa_Boolean firesEvents,  //!< [in] Flag indicating if the NodeManager should activate the EventManager
    OpcUa_Int32 nHashTableSize) //!< [in] Size of the hash table. The default value is 10.007.<br>
                                //   The size can be smaller than the total number of nodes managed
                                //   by the node manager but it is better for the performance to avoid
                                //   collisions with a large size.
                                //   Prefered sizes are 1.009, 10.007, 100.003, 1.000.003, 10.000.019.
: NodeManagerBase("http://opcfoundation.org/UA/CNC/", firesEvents, nHashTableSize)
{
    m_defaultLocaleId         = "en";
}

/** Finish start up in derived class after starting up base class.
*/
UaStatus NodeManagerCNCBase::afterStartUp()
{
    UaStatus ret;

    s_namespaceIndex = getNameSpaceIndex();

    OpcUaCnc::DataTypes::registerStructuredTypes();

    ret = createDataTypeNodes();
    UA_ASSERT(ret.isGood());
    ret = createReferenceTypeNodes();
    UA_ASSERT(ret.isGood());

    // ------------------------------------------------------------
    // Create object type nodes
    // ------------------------------------------------------------
    CncAlarmType::createTypes();
    CncMessageType::createTypes();
    CncAxisListType::createTypes();
    CncChannelListType::createTypes();
    CncComponentType::createTypes();
    CncChannelType::createTypes();
    CncDriveType::createTypes();
    CncAxisType::createTypes();
    CncSpindleType::createTypes();
    CncInterfaceType::createTypes();
    CncSpindleListType::createTypes();
    // ------------------------------------------------------------

    // ------------------------------------------------------------
    // Create variable type nodes
    // ------------------------------------------------------------
    CncPositionVariableType::createTypes();
    // ------------------------------------------------------------

    XmlUaNodeFactoryManager factory;
    OpcUaCnc::InstanceFactoryCNC factoryCNC(getTypeNamespace());
    factory.addNamespace(&factoryCNC);

    ret = addObjectsFolderCncInterface(factory);
    UA_ASSERT(ret.isGood());
    if (ret.isBad()) return ret;
    ret = addAdditionalHierarchicalReferences();

    return ret;
}

/** Static method to get the namespace index for the type namespace managed by this class.
 */
OpcUa_UInt16 NodeManagerCNCBase::getTypeNamespace()
{
    return s_namespaceIndex;
}

/** Helper function to create data type related nodes.
*/
UaStatus NodeManagerCNCBase::createDataTypeNodes()
{
    UaStatus               ret;
    UaVariant              defaultValue;
    UaGenericDataType*     pDataType;
    OpcUa::BaseObjectTypeGeneric *pObject;
    UaLocalizedTextArray   ltEnumStrings;
    UaString               sTemp;
    OpcUa::PropertyType*   pEnumString;
    // Enumerations

    // CncAxisStatus
    pDataType = new UaGenericDataType(
                    UaNodeId(OpcUaCncId_CncAxisStatus, getNameSpaceIndex()),
                    UaQualifiedName("CncAxisStatus", getNameSpaceIndex()),
                    UaLocalizedText("", "CncAxisStatus"),
                    UaLocalizedText("", ""),
                    OpcUa_False);
    ret = addNodeAndReference(OpcUaId_Enumeration, pDataType, OpcUaId_HasSubtype);
    UA_ASSERT(ret.isGood());
    // EnumStrings
    ltEnumStrings.create(3);
    sTemp = "InPosition";
    sTemp.copyTo(&ltEnumStrings[0].Text);
    sTemp = "Moving";
    sTemp.copyTo(&ltEnumStrings[1].Text);
    sTemp = "Parked";
    sTemp.copyTo(&ltEnumStrings[2].Text);

    defaultValue.setLocalizedTextArray(ltEnumStrings);
    pEnumString = new OpcUa::PropertyType(UaNodeId(OpcUaCncId_CncAxisStatus_EnumStrings, getNameSpaceIndex()), "EnumStrings", 0, defaultValue, 1, this);
    defaultValue.clear();
    ret = addNodeAndReference(pDataType, pEnumString, OpcUaId_HasProperty);
    UA_ASSERT(ret.isGood());

    // CncChannelProgramStatus
    pDataType = new UaGenericDataType(
                    UaNodeId(OpcUaCncId_CncChannelProgramStatus, getNameSpaceIndex()),
                    UaQualifiedName("CncChannelProgramStatus", getNameSpaceIndex()),
                    UaLocalizedText("", "CncChannelProgramStatus"),
                    UaLocalizedText("", ""),
                    OpcUa_False);
    ret = addNodeAndReference(OpcUaId_Enumeration, pDataType, OpcUaId_HasSubtype);
    UA_ASSERT(ret.isGood());
    // EnumStrings
    ltEnumStrings.create(5);
    sTemp = "Stopped";
    sTemp.copyTo(&ltEnumStrings[0].Text);
    sTemp = "Running";
    sTemp.copyTo(&ltEnumStrings[1].Text);
    sTemp = "Waiting";
    sTemp.copyTo(&ltEnumStrings[2].Text);
    sTemp = "Interrupted";
    sTemp.copyTo(&ltEnumStrings[3].Text);
    sTemp = "Canceled";
    sTemp.copyTo(&ltEnumStrings[4].Text);

    defaultValue.setLocalizedTextArray(ltEnumStrings);
    pEnumString = new OpcUa::PropertyType(UaNodeId(OpcUaCncId_CncChannelProgramStatus_EnumStrings, getNameSpaceIndex()), "EnumStrings", 0, defaultValue, 1, this);
    defaultValue.clear();
    ret = addNodeAndReference(pDataType, pEnumString, OpcUaId_HasProperty);
    UA_ASSERT(ret.isGood());

    // CncChannelStatus
    pDataType = new UaGenericDataType(
                    UaNodeId(OpcUaCncId_CncChannelStatus, getNameSpaceIndex()),
                    UaQualifiedName("CncChannelStatus", getNameSpaceIndex()),
                    UaLocalizedText("", "CncChannelStatus"),
                    UaLocalizedText("", ""),
                    OpcUa_False);
    ret = addNodeAndReference(OpcUaId_Enumeration, pDataType, OpcUaId_HasSubtype);
    UA_ASSERT(ret.isGood());
    // EnumStrings
    ltEnumStrings.create(3);
    sTemp = "Active";
    sTemp.copyTo(&ltEnumStrings[0].Text);
    sTemp = "Interrupted";
    sTemp.copyTo(&ltEnumStrings[1].Text);
    sTemp = "Reset";
    sTemp.copyTo(&ltEnumStrings[2].Text);

    defaultValue.setLocalizedTextArray(ltEnumStrings);
    pEnumString = new OpcUa::PropertyType(UaNodeId(OpcUaCncId_CncChannelStatus_EnumStrings, getNameSpaceIndex()), "EnumStrings", 0, defaultValue, 1, this);
    defaultValue.clear();
    ret = addNodeAndReference(pDataType, pEnumString, OpcUaId_HasProperty);
    UA_ASSERT(ret.isGood());

    // CncOperationMode
    pDataType = new UaGenericDataType(
                    UaNodeId(OpcUaCncId_CncOperationMode, getNameSpaceIndex()),
                    UaQualifiedName("CncOperationMode", getNameSpaceIndex()),
                    UaLocalizedText("", "CncOperationMode"),
                    UaLocalizedText("", ""),
                    OpcUa_False);
    ret = addNodeAndReference(OpcUaId_Enumeration, pDataType, OpcUaId_HasSubtype);
    UA_ASSERT(ret.isGood());
    // EnumStrings
    ltEnumStrings.create(3);
    sTemp = "Manual";
    sTemp.copyTo(&ltEnumStrings[0].Text);
    sTemp = "MDA";
    sTemp.copyTo(&ltEnumStrings[1].Text);
    sTemp = "Automatic";
    sTemp.copyTo(&ltEnumStrings[2].Text);

    defaultValue.setLocalizedTextArray(ltEnumStrings);
    pEnumString = new OpcUa::PropertyType(UaNodeId(OpcUaCncId_CncOperationMode_EnumStrings, getNameSpaceIndex()), "EnumStrings", 0, defaultValue, 1, this);
    defaultValue.clear();
    ret = addNodeAndReference(pDataType, pEnumString, OpcUaId_HasProperty);
    UA_ASSERT(ret.isGood());

    // CncSpindleStatus
    pDataType = new UaGenericDataType(
                    UaNodeId(OpcUaCncId_CncSpindleStatus, getNameSpaceIndex()),
                    UaQualifiedName("CncSpindleStatus", getNameSpaceIndex()),
                    UaLocalizedText("", "CncSpindleStatus"),
                    UaLocalizedText("", ""),
                    OpcUa_False);
    ret = addNodeAndReference(OpcUaId_Enumeration, pDataType, OpcUaId_HasSubtype);
    UA_ASSERT(ret.isGood());
    // EnumStrings
    ltEnumStrings.create(5);
    sTemp = "Stopped";
    sTemp.copyTo(&ltEnumStrings[0].Text);
    sTemp = "InTargetArea";
    sTemp.copyTo(&ltEnumStrings[1].Text);
    sTemp = "Accelerating";
    sTemp.copyTo(&ltEnumStrings[2].Text);
    sTemp = "Decelerating";
    sTemp.copyTo(&ltEnumStrings[3].Text);
    sTemp = "Parked";
    sTemp.copyTo(&ltEnumStrings[4].Text);

    defaultValue.setLocalizedTextArray(ltEnumStrings);
    pEnumString = new OpcUa::PropertyType(UaNodeId(OpcUaCncId_CncSpindleStatus_EnumStrings, getNameSpaceIndex()), "EnumStrings", 0, defaultValue, 1, this);
    defaultValue.clear();
    ret = addNodeAndReference(pDataType, pEnumString, OpcUaId_HasProperty);
    UA_ASSERT(ret.isGood());

    // CncSpindleTurnDirection
    pDataType = new UaGenericDataType(
                    UaNodeId(OpcUaCncId_CncSpindleTurnDirection, getNameSpaceIndex()),
                    UaQualifiedName("CncSpindleTurnDirection", getNameSpaceIndex()),
                    UaLocalizedText("", "CncSpindleTurnDirection"),
                    UaLocalizedText("", ""),
                    OpcUa_False);
    ret = addNodeAndReference(OpcUaId_Enumeration, pDataType, OpcUaId_HasSubtype);
    UA_ASSERT(ret.isGood());
    // EnumStrings
    ltEnumStrings.create(3);
    sTemp = "None";
    sTemp.copyTo(&ltEnumStrings[0].Text);
    sTemp = "CW";
    sTemp.copyTo(&ltEnumStrings[1].Text);
    sTemp = "CCW";
    sTemp.copyTo(&ltEnumStrings[2].Text);

    defaultValue.setLocalizedTextArray(ltEnumStrings);
    pEnumString = new OpcUa::PropertyType(UaNodeId(OpcUaCncId_CncSpindleTurnDirection_EnumStrings, getNameSpaceIndex()), "EnumStrings", 0, defaultValue, 1, this);
    defaultValue.clear();
    ret = addNodeAndReference(pDataType, pEnumString, OpcUaId_HasProperty);
    UA_ASSERT(ret.isGood());

    // Structured Types

    // CncPositionType
    pDataType = new UaGenericDataType(
                    UaNodeId(OpcUaCncId_CncPositionType, getNameSpaceIndex()),
                    UaQualifiedName("CncPositionType", getNameSpaceIndex()),
                    UaLocalizedText("", "CncPositionType"),
                    UaLocalizedText("", ""),
                    OpcUa_False);
    ret = addNodeAndReference(UaNodeId(OpcUaId_Structure), pDataType, OpcUaId_HasSubtype);
    UA_ASSERT(ret.isGood());

    pObject = new OpcUa::BaseObjectTypeGeneric(
                    UaNodeId(OpcUaCncId_CncPositionType_Encoding_DefaultBinary, getNameSpaceIndex()),
                    "Default Binary",
                    0,
                    this);
    pObject->setTypeDefinitionId(OpcUaId_DataTypeEncodingType);

    ret = addNodeAndReference(pDataType, pObject, OpcUaId_HasEncoding);
    UA_ASSERT(ret.isGood());

    // Data Type Dictionary
    UaByteString bsDictionary(2263, (OpcUa_Byte*)"\074\157\160\143\072\124\171\160\145\104\151\143\164\151\157\156\141\162\171\040\170\155\154\156\163\072\170\163\151\075\042\150"
        "\164\164\160\072\057\057\167\167\167\056\167\063\056\157\162\147\057\062\060\060\061\057\130\115\114\123\143\150\145\155\141\055"
        "\151\156\163\164\141\156\143\145\042\040\170\155\154\156\163\072\164\156\163\075\042\150\164\164\160\072\057\057\157\160\143\146"
        "\157\165\156\144\141\164\151\157\156\056\157\162\147\057\125\101\057\103\116\103\057\042\040\104\145\146\141\165\154\164\102\171"
        "\164\145\117\162\144\145\162\075\042\114\151\164\164\154\145\105\156\144\151\141\156\042\040\170\155\154\156\163\072\157\160\143"
        "\075\042\150\164\164\160\072\057\057\157\160\143\146\157\165\156\144\141\164\151\157\156\056\157\162\147\057\102\151\156\141\162"
        "\171\123\143\150\145\155\141\057\042\040\170\155\154\156\163\072\165\141\075\042\150\164\164\160\072\057\057\157\160\143\146\157"
        "\165\156\144\141\164\151\157\156\056\157\162\147\057\125\101\057\042\040\124\141\162\147\145\164\116\141\155\145\163\160\141\143"
        "\145\075\042\150\164\164\160\072\057\057\157\160\143\146\157\165\156\144\141\164\151\157\156\056\157\162\147\057\125\101\057\103"
        "\116\103\057\042\076\012\040\074\157\160\143\072\111\155\160\157\162\164\040\116\141\155\145\163\160\141\143\145\075\042\150\164"
        "\164\160\072\057\057\157\160\143\146\157\165\156\144\141\164\151\157\156\056\157\162\147\057\125\101\057\042\057\076\012\040\074"
        "\157\160\143\072\123\164\162\165\143\164\165\162\145\144\124\171\160\145\040\102\141\163\145\124\171\160\145\075\042\165\141\072"
        "\105\170\164\145\156\163\151\157\156\117\142\152\145\143\164\042\040\116\141\155\145\075\042\103\156\143\120\157\163\151\164\151"
        "\157\156\124\171\160\145\042\076\012\040\040\074\157\160\143\072\106\151\145\154\144\040\124\171\160\145\116\141\155\145\075\042"
        "\157\160\143\072\104\157\165\142\154\145\042\040\116\141\155\145\075\042\101\143\164\120\157\163\042\057\076\012\040\040\074\157"
        "\160\143\072\106\151\145\154\144\040\124\171\160\145\116\141\155\145\075\042\157\160\143\072\104\157\165\142\154\145\042\040\116"
        "\141\155\145\075\042\103\155\144\120\157\163\042\057\076\012\040\040\074\157\160\143\072\106\151\145\154\144\040\124\171\160\145"
        "\116\141\155\145\075\042\157\160\143\072\104\157\165\142\154\145\042\040\116\141\155\145\075\042\122\145\155\104\151\163\164\042"
        "\057\076\012\040\074\057\157\160\143\072\123\164\162\165\143\164\165\162\145\144\124\171\160\145\076\012\040\074\157\160\143\072"
        "\105\156\165\155\145\162\141\164\145\144\124\171\160\145\040\114\145\156\147\164\150\111\156\102\151\164\163\075\042\063\062\042"
        "\040\116\141\155\145\075\042\103\156\143\101\170\151\163\123\164\141\164\165\163\042\076\012\040\040\074\157\160\143\072\105\156"
        "\165\155\145\162\141\164\145\144\126\141\154\165\145\040\116\141\155\145\075\042\111\156\120\157\163\151\164\151\157\156\042\040"
        "\126\141\154\165\145\075\042\060\042\057\076\012\040\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145\144\126\141\154"
        "\165\145\040\116\141\155\145\075\042\115\157\166\151\156\147\042\040\126\141\154\165\145\075\042\061\042\057\076\012\040\040\074"
        "\157\160\143\072\105\156\165\155\145\162\141\164\145\144\126\141\154\165\145\040\116\141\155\145\075\042\120\141\162\153\145\144"
        "\042\040\126\141\154\165\145\075\042\062\042\057\076\012\040\074\057\157\160\143\072\105\156\165\155\145\162\141\164\145\144\124"
        "\171\160\145\076\012\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145\144\124\171\160\145\040\114\145\156\147\164\150"
        "\111\156\102\151\164\163\075\042\063\062\042\040\116\141\155\145\075\042\103\156\143\103\150\141\156\156\145\154\120\162\157\147"
        "\162\141\155\123\164\141\164\165\163\042\076\012\040\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145\144\126\141\154"
        "\165\145\040\116\141\155\145\075\042\123\164\157\160\160\145\144\042\040\126\141\154\165\145\075\042\060\042\057\076\012\040\040"
        "\074\157\160\143\072\105\156\165\155\145\162\141\164\145\144\126\141\154\165\145\040\116\141\155\145\075\042\122\165\156\156\151"
        "\156\147\042\040\126\141\154\165\145\075\042\061\042\057\076\012\040\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145"
        "\144\126\141\154\165\145\040\116\141\155\145\075\042\127\141\151\164\151\156\147\042\040\126\141\154\165\145\075\042\062\042\057"
        "\076\012\040\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145\144\126\141\154\165\145\040\116\141\155\145\075\042\111"
        "\156\164\145\162\162\165\160\164\145\144\042\040\126\141\154\165\145\075\042\063\042\057\076\012\040\040\074\157\160\143\072\105"
        "\156\165\155\145\162\141\164\145\144\126\141\154\165\145\040\116\141\155\145\075\042\103\141\156\143\145\154\145\144\042\040\126"
        "\141\154\165\145\075\042\064\042\057\076\012\040\074\057\157\160\143\072\105\156\165\155\145\162\141\164\145\144\124\171\160\145"
        "\076\012\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145\144\124\171\160\145\040\114\145\156\147\164\150\111\156\102"
        "\151\164\163\075\042\063\062\042\040\116\141\155\145\075\042\103\156\143\103\150\141\156\156\145\154\123\164\141\164\165\163\042"
        "\076\012\040\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145\144\126\141\154\165\145\040\116\141\155\145\075\042\101"
        "\143\164\151\166\145\042\040\126\141\154\165\145\075\042\060\042\057\076\012\040\040\074\157\160\143\072\105\156\165\155\145\162"
        "\141\164\145\144\126\141\154\165\145\040\116\141\155\145\075\042\111\156\164\145\162\162\165\160\164\145\144\042\040\126\141\154"
        "\165\145\075\042\061\042\057\076\012\040\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145\144\126\141\154\165\145\040"
        "\116\141\155\145\075\042\122\145\163\145\164\042\040\126\141\154\165\145\075\042\062\042\057\076\012\040\074\057\157\160\143\072"
        "\105\156\165\155\145\162\141\164\145\144\124\171\160\145\076\012\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145\144"
        "\124\171\160\145\040\114\145\156\147\164\150\111\156\102\151\164\163\075\042\063\062\042\040\116\141\155\145\075\042\103\156\143"
        "\117\160\145\162\141\164\151\157\156\115\157\144\145\042\076\012\040\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145"
        "\144\126\141\154\165\145\040\116\141\155\145\075\042\115\141\156\165\141\154\042\040\126\141\154\165\145\075\042\060\042\057\076"
        "\012\040\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145\144\126\141\154\165\145\040\116\141\155\145\075\042\115\104"
        "\101\042\040\126\141\154\165\145\075\042\061\042\057\076\012\040\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145\144"
        "\126\141\154\165\145\040\116\141\155\145\075\042\101\165\164\157\155\141\164\151\143\042\040\126\141\154\165\145\075\042\062\042"
        "\057\076\012\040\074\057\157\160\143\072\105\156\165\155\145\162\141\164\145\144\124\171\160\145\076\012\040\074\157\160\143\072"
        "\105\156\165\155\145\162\141\164\145\144\124\171\160\145\040\114\145\156\147\164\150\111\156\102\151\164\163\075\042\063\062\042"
        "\040\116\141\155\145\075\042\103\156\143\123\160\151\156\144\154\145\123\164\141\164\165\163\042\076\012\040\040\074\157\160\143"
        "\072\105\156\165\155\145\162\141\164\145\144\126\141\154\165\145\040\116\141\155\145\075\042\123\164\157\160\160\145\144\042\040"
        "\126\141\154\165\145\075\042\060\042\057\076\012\040\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145\144\126\141\154"
        "\165\145\040\116\141\155\145\075\042\111\156\124\141\162\147\145\164\101\162\145\141\042\040\126\141\154\165\145\075\042\061\042"
        "\057\076\012\040\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145\144\126\141\154\165\145\040\116\141\155\145\075\042"
        "\101\143\143\145\154\145\162\141\164\151\156\147\042\040\126\141\154\165\145\075\042\062\042\057\076\012\040\040\074\157\160\143"
        "\072\105\156\165\155\145\162\141\164\145\144\126\141\154\165\145\040\116\141\155\145\075\042\104\145\143\145\154\145\162\141\164"
        "\151\156\147\042\040\126\141\154\165\145\075\042\063\042\057\076\012\040\040\074\157\160\143\072\105\156\165\155\145\162\141\164"
        "\145\144\126\141\154\165\145\040\116\141\155\145\075\042\120\141\162\153\145\144\042\040\126\141\154\165\145\075\042\064\042\057"
        "\076\012\040\074\057\157\160\143\072\105\156\165\155\145\162\141\164\145\144\124\171\160\145\076\012\040\074\157\160\143\072\105"
        "\156\165\155\145\162\141\164\145\144\124\171\160\145\040\114\145\156\147\164\150\111\156\102\151\164\163\075\042\063\062\042\040"
        "\116\141\155\145\075\042\103\156\143\123\160\151\156\144\154\145\124\165\162\156\104\151\162\145\143\164\151\157\156\042\076\012"
        "\040\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145\144\126\141\154\165\145\040\116\141\155\145\075\042\116\157\156"
        "\145\042\040\126\141\154\165\145\075\042\060\042\057\076\012\040\040\074\157\160\143\072\105\156\165\155\145\162\141\164\145\144"
        "\126\141\154\165\145\040\116\141\155\145\075\042\103\127\042\040\126\141\154\165\145\075\042\061\042\057\076\012\040\040\074\157"
        "\160\143\072\105\156\165\155\145\162\141\164\145\144\126\141\154\165\145\040\116\141\155\145\075\042\103\103\127\042\040\126\141"
        "\154\165\145\075\042\062\042\057\076\012\040\074\057\157\160\143\072\105\156\165\155\145\162\141\164\145\144\124\171\160\145\076"
        "\012\074\057\157\160\143\072\124\171\160\145\104\151\143\164\151\157\156\141\162\171\076\012");
    defaultValue.setByteString(bsDictionary, OpcUa_True);
    OpcUa::BaseVariableTypeGeneric *pBaseVariableType = new OpcUa::BaseVariableTypeGeneric(
        UaNodeId(OpcUaCncId_BinarySchema, getNameSpaceIndex()),
        "TypeDictionary",
        getNameSpaceIndex(),
        defaultValue,
        OpcUa_AccessLevels_CurrentRead,
        this);
    defaultValue.clear();
    pBaseVariableType->setTypeDefinitionId(OpcUaId_DataTypeDictionaryType);

    ret = addNodeAndReference(OpcUaId_OPCBinarySchema_TypeSystem, pBaseVariableType, OpcUaId_HasComponent);
    UA_ASSERT(ret.isGood());

    defaultValue.setString("CncPositionType");
    OpcUa::BaseVariableTypeGeneric *pOpcUaCncId_BinarySchema_CncPositionType = new OpcUa::BaseVariableTypeGeneric(
        UaNodeId(OpcUaCncId_BinarySchema_CncPositionType, getNameSpaceIndex()),
        "CncPositionType",
        getNameSpaceIndex(),
        defaultValue,
        OpcUa_AccessLevels_CurrentRead,
        this);
    defaultValue.clear();
    pOpcUaCncId_BinarySchema_CncPositionType->setTypeDefinitionId(OpcUaId_DataTypeDescriptionType);

    ret = addNodeAndReference(pBaseVariableType, pOpcUaCncId_BinarySchema_CncPositionType, OpcUaId_HasComponent);
    UA_ASSERT(ret.isGood());
    ret = addUaReference(UaNodeId(OpcUaCncId_CncPositionType_Encoding_DefaultBinary, getNameSpaceIndex()), pOpcUaCncId_BinarySchema_CncPositionType->nodeId(), OpcUaId_HasDescription);
    UA_ASSERT(ret.isGood());

    defaultValue.setString("http://opcfoundation.org/UA/CNC/");
    OpcUa::PropertyType *pOpcUaCncId_BinarySchema_NamespaceUri = new OpcUa::PropertyType(
        UaNodeId(OpcUaCncId_BinarySchema_NamespaceUri, getNameSpaceIndex()),
        "NamespaceUri",
        0,
        defaultValue,
        OpcUa_AccessLevels_CurrentRead,
        this);
    defaultValue.clear();

    ret = addNodeAndReference(pBaseVariableType, pOpcUaCncId_BinarySchema_NamespaceUri, OpcUaId_HasProperty);
    UA_ASSERT(ret.isGood());
    return ret;
}

/** Helper function to create reference type related nodes.
*/
UaStatus NodeManagerCNCBase::createReferenceTypeNodes()
{
    UaStatus                ret;

    return ret;
}

UaStatus NodeManagerCNCBase::addObjectsFolderCncInterface(const XmlUaNodeFactoryManager &factory)
{
    UaStatus ret;
    UaVariant defaultValue;
    UaNodeId referenceTypeId;
    UaNodeId parentEventNodeId;
    UaBase::Object *pCncInterface = new UaBase::Object(UaNodeId(OpcUaCncId_CncInterface, getTypeNamespace()));
    pCncInterface->setBrowseName(UaQualifiedName("CncInterface", getTypeNamespace()));
    pCncInterface->setDisplayName(UaLocalizedText("", "CncInterface"));
    UaBase::FullReference reference;
    // CncAxisList
    UaBase::Object *pCncAxisList = new UaBase::Object(UaNodeId(OpcUaCncId_CncInterface_CncAxisList, getTypeNamespace()));
    pCncAxisList->setBrowseName(UaQualifiedName("CncAxisList", getTypeNamespace()));
    pCncAxisList->setDisplayName(UaLocalizedText("", "CncAxisList"));
    pCncAxisList->setTypeDefinitionId(UaNodeId(OpcUaCncId_CncAxisListType, getNameSpaceIndex()));
    reference.setReferenceTypeId(OpcUaId_HasComponent);
    reference.setSourceNode(pCncInterface);
    reference.setTargetNode(pCncAxisList);
    pCncInterface->addFullReference(reference);
    pCncAxisList->addFullReference(reference);
    pCncAxisList->setParentNodeId(pCncInterface->nodeId());

    // CncChannelList
    UaBase::Object *pCncChannelList = new UaBase::Object(UaNodeId(OpcUaCncId_CncInterface_CncChannelList, getTypeNamespace()));
    pCncChannelList->setBrowseName(UaQualifiedName("CncChannelList", getTypeNamespace()));
    pCncChannelList->setDisplayName(UaLocalizedText("", "CncChannelList"));
    pCncChannelList->setTypeDefinitionId(UaNodeId(OpcUaCncId_CncChannelListType, getNameSpaceIndex()));
    reference.setReferenceTypeId(OpcUaId_HasComponent);
    reference.setSourceNode(pCncInterface);
    reference.setTargetNode(pCncChannelList);
    pCncInterface->addFullReference(reference);
    pCncChannelList->addFullReference(reference);
    pCncChannelList->setParentNodeId(pCncInterface->nodeId());

    // CncSpindleList
    UaBase::Object *pCncSpindleList = new UaBase::Object(UaNodeId(OpcUaCncId_CncInterface_CncSpindleList, getTypeNamespace()));
    pCncSpindleList->setBrowseName(UaQualifiedName("CncSpindleList", getTypeNamespace()));
    pCncSpindleList->setDisplayName(UaLocalizedText("", "CncSpindleList"));
    pCncSpindleList->setTypeDefinitionId(UaNodeId(OpcUaCncId_CncSpindleListType, getNameSpaceIndex()));
    reference.setReferenceTypeId(OpcUaId_HasComponent);
    reference.setSourceNode(pCncInterface);
    reference.setTargetNode(pCncSpindleList);
    pCncInterface->addFullReference(reference);
    pCncSpindleList->addFullReference(reference);
    pCncSpindleList->setParentNodeId(pCncInterface->nodeId());

    // Revision
    UaBase::Variable *pRevision = new UaBase::Variable(UaNodeId(OpcUaCncId_CncInterface_Revision, getTypeNamespace()));
    pRevision->setBrowseName(UaQualifiedName("Revision", getTypeNamespace()));
    pRevision->setDisplayName(UaLocalizedText("", "Revision"));
    pRevision->setAccessLevel(1);
    pRevision->setUserAccessLevel(1);
    pRevision->setValueRank(-1);
    pRevision->setDataTypeId(OpcUaId_String);
    defaultValue.setString("");
    pRevision->setValue(defaultValue);
    pRevision->setTypeDefinitionId(OpcUaId_PropertyType);
    reference.setReferenceTypeId(OpcUaId_HasProperty);
    reference.setSourceNode(pCncInterface);
    reference.setTargetNode(pRevision);
    pCncInterface->addFullReference(reference);
    pRevision->addFullReference(reference);
    pRevision->setParentNodeId(pCncInterface->nodeId());

    // VendorName
    UaBase::Variable *pVendorName = new UaBase::Variable(UaNodeId(OpcUaCncId_CncInterface_VendorName, getTypeNamespace()));
    pVendorName->setBrowseName(UaQualifiedName("VendorName", getTypeNamespace()));
    pVendorName->setDisplayName(UaLocalizedText("", "VendorName"));
    pVendorName->setAccessLevel(1);
    pVendorName->setUserAccessLevel(1);
    pVendorName->setValueRank(-1);
    pVendorName->setDataTypeId(OpcUaId_String);
    defaultValue.setString("");
    pVendorName->setValue(defaultValue);
    pVendorName->setTypeDefinitionId(OpcUaId_PropertyType);
    reference.setReferenceTypeId(OpcUaId_HasProperty);
    reference.setSourceNode(pCncInterface);
    reference.setTargetNode(pVendorName);
    pCncInterface->addFullReference(reference);
    pVendorName->addFullReference(reference);
    pVendorName->setParentNodeId(pCncInterface->nodeId());

    // VendorRevision
    UaBase::Variable *pVendorRevision = new UaBase::Variable(UaNodeId(OpcUaCncId_CncInterface_VendorRevision, getTypeNamespace()));
    pVendorRevision->setBrowseName(UaQualifiedName("VendorRevision", getTypeNamespace()));
    pVendorRevision->setDisplayName(UaLocalizedText("", "VendorRevision"));
    pVendorRevision->setAccessLevel(1);
    pVendorRevision->setUserAccessLevel(1);
    pVendorRevision->setValueRank(-1);
    pVendorRevision->setDataTypeId(OpcUaId_String);
    defaultValue.setString("");
    pVendorRevision->setValue(defaultValue);
    pVendorRevision->setTypeDefinitionId(OpcUaId_PropertyType);
    reference.setReferenceTypeId(OpcUaId_HasProperty);
    reference.setSourceNode(pCncInterface);
    reference.setTargetNode(pVendorRevision);
    pCncInterface->addFullReference(reference);
    pVendorRevision->addFullReference(reference);
    pVendorRevision->setParentNodeId(pCncInterface->nodeId());

    // Version
    UaBase::Variable *pVersion = new UaBase::Variable(UaNodeId(OpcUaCncId_CncInterface_Version, getTypeNamespace()));
    pVersion->setBrowseName(UaQualifiedName("Version", getTypeNamespace()));
    pVersion->setDisplayName(UaLocalizedText("", "Version"));
    pVersion->setAccessLevel(1);
    pVersion->setUserAccessLevel(1);
    pVersion->setValueRank(-1);
    pVersion->setDataTypeId(OpcUaId_String);
    defaultValue.setString("");
    pVersion->setValue(defaultValue);
    pVersion->setTypeDefinitionId(OpcUaId_PropertyType);
    reference.setReferenceTypeId(OpcUaId_HasProperty);
    reference.setSourceNode(pCncInterface);
    reference.setTargetNode(pVersion);
    pCncInterface->addFullReference(reference);
    pVersion->addFullReference(reference);
    pVersion->setParentNodeId(pCncInterface->nodeId());

    OpcUaCnc::CncInterfaceType *pObjectType = new OpcUaCnc::CncInterfaceType(pCncInterface, (XmlUaNodeFactoryManager*) &factory, this);
    pObjectType->useAccessInfoFromType();

    referenceTypeId = OpcUaId_Organizes;
    ret = addNodeAndReference(OpcUaId_ObjectsFolder, pObjectType, referenceTypeId);
    UA_ASSERT(ret.isGood());
    ret = addUaReference(pObjectType->nodeId(), UaNodeId(OpcUaCncId_CncAlarmType, getNameSpaceIndex()), OpcUaId_GeneratesEvent);
    UA_ASSERT(ret.isGood());
    ret = addUaReference(pObjectType->nodeId(), UaNodeId(OpcUaCncId_CncMessageType, getNameSpaceIndex()), OpcUaId_GeneratesEvent);
    UA_ASSERT(ret.isGood());
    delete pCncAxisList;
    delete pCncChannelList;
    delete pCncSpindleList;
    delete pRevision;
    delete pVendorName;
    delete pVendorRevision;
    delete pVersion;
    delete pCncInterface;
    return ret;
}


UaStatus NodeManagerCNCBase::addAdditionalHierarchicalReferences()
{
    UaStatus ret;

    return ret;
}


} // End namespace for the UA information model http://opcfoundation.org/UA/CNC/



