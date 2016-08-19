/******************************************************************************
** thesiscnc_identifiers.h
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

#ifndef __THESISCNC_IDENTIFIERS_H__
#define __THESISCNC_IDENTIFIERS_H__

#define ThesisCNC_NameSpaceUri "http://cs.hs-rm.de/mierswa_thesis/CNC/"

#include "version_coremodule.h"
#if (PROD_MAJOR != 1 || (PROD_MAJOR == 1 && PROD_MINOR < 5) || (PROD_MAJOR == 1 && PROD_MINOR == 5 && PROD_MINOR2 < 0))
#if ! (PROD_MAJOR == 1 && PROD_MINOR == 3 && PROD_MINOR2 == 0 && CPP_SDK_BUILD == 193)
#error The code is generated for SDK version 1.5.0
#endif
#endif

/*============================================================================
* ObjectType Identifiers
 *===========================================================================*/

/*============================================================================
* VariableType Identifiers
 *===========================================================================*/

/*============================================================================
* DataType Identifiers
 *===========================================================================*/

/*============================================================================
* ReferenceType Identifiers
 *===========================================================================*/

/*============================================================================
* Object Identifiers
 *===========================================================================*/

/*============================================================================
* Variable Identifiers
 *===========================================================================*/

/*============================================================================
* Method Identifiers
 *===========================================================================*/


#ifdef _WIN32
#ifdef _UAMODELS_BUILD_DLL
# define UAMODELS_EXPORT __declspec(dllexport)
#elif defined _UAMODELS_USE_DLL
# define UAMODELS_EXPORT __declspec(dllimport)
#else
# define UAMODELS_EXPORT
#endif
#else
# define UAMODELS_EXPORT
#endif

///////////////////////////////////////////////
// Private Class Hiding Macros
#define THESISCNC_DECLARE_PRIVATE(Class) \
    inline Class##Private* d_func() { return reinterpret_cast<Class##Private *>(d_ptr); } \
    inline const Class##Private* d_func() const { return reinterpret_cast<const Class##Private *>(d_ptr); } \
    friend class Class##Private; \
    Class##Private *d_ptr;

#define THESISCNC_DECLARE_PUBLIC(Class) \
    inline Class* q_func() { return static_cast<Class *>(q_ptr); } \
    inline const Class* q_func() const { return static_cast<const Class *>(q_ptr); } \
    friend class Class;

#define THESISCNC_D(Class) Class##Private * const d = d_func()
#define THESISCNC_CD(Class) const Class##Private * const d = d_func()
#define THESISCNC_Q(Class) Class * const q = q_func()

#endif // __THESISCNC_IDENTIFIERS_H__

