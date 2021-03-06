###############################################################################
#
# Copyright (C) 2005-2014 Unified Automation GmbH. All Rights Reserved.
# Web: http://www.unified-automation.com
# This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
# WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
# This software is licensed under the proprietary Software License Agreement
# (SLA) of Unified Automation GmbH. You are not allowed to publish, distribute,
# sublicense, and/or sell the Source Code of this Software.
# Get full License Agreement at: license@www.unified-automation.com
#
# Configures the uastack submodule.
###############################################################################

# SDK platform variable
if (${CMAKE_SYSTEM_NAME} MATCHES "^(Windows|WindowsCE|WindowsIoT)$")
    set(UASDK_PLATFORM "win32")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(UASDK_PLATFORM "linux")
elseif (vxWorks)
    set(UASDK_PLATFORM "vxworks")
elseif (QNXNTO)
    set(UASDK_PLATFORM "qnx")
elseif (APPLE)
    set(UASDK_PLATFORM "apple")
elseif (EUROS)
    set(UASDK_PLATFORM "euros")
else ()
    message(FATAL_ERROR "Unknown SDK platform!")
endif ()
set(UASDK_PLATFORM ${UASDK_PLATFORM} CACHE STRING "SDK platform chosen by the user at CMake configure time")
unset(UASDK_PLATFORM)
set(UASDK_PLATFORM_VALUES "win32;codesys;euros;intime;linux;qnx;vxworks;apple")
set_property(CACHE UASDK_PLATFORM PROPERTY STRINGS ${UASDK_PLATFORM_VALUES})
#message(STATUS "Selected SDK platform ${UASDK_PLATFORM}")

if (NOT CMAKE_CURRENT_LIST_DIR)
    get_filename_component(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
endif ()

if (DEFINED uabase_SOURCE_DIR)
    set(_BASE_DIR                   ${CMAKE_CURRENT_LIST_DIR}/../src)
    set(UABASE_INCLUDE              ${_BASE_DIR}/uabase/uabasecpp
                                    ${_BASE_DIR}/uabase/uabasecpp/platforms/${UASDK_PLATFORM}
                                    ${uabase_BINARY_DIR})
    set(UAPKI_INCLUDE               ${_BASE_DIR}/uabase/uapkicpp)
    set(UAXMLPARSER_INCLUDE         ${_BASE_DIR}/uabase/xmlparser)
    set(UACOREMODULE_INCLUDE        ${_BASE_DIR}/uaserver/uaservercpp/coremodule
                                    ${coremodule_BINARY_DIR})
    set(UAMODULE_INCLUDE            ${_BASE_DIR}/uaserver/uaservercpp/uamodule)
    set(PUBSUBMODULE_INCLUDE        ${_BASE_DIR}/uaserver/uaservercpp/pubsubmodule)
    set(UAMODELS_INCLUDE            ${_BASE_DIR}/uaserver/uaservercpp/uamodels)
    set(UACLIENT_INCLUDE            ${_BASE_DIR}/uaclient/uaclientcpp)
    set(COMBASE_INCLUDE             ${_BASE_DIR}/uaserver/uaservercpp/combase)
    set(COMBASE_CONFIG_INCLUDE      ${_BASE_DIR}/uaserver/uaservercpp/combase/DCOM_Config)
    set(COMDAMODULE_INCLUDE         ${_BASE_DIR}/uaserver/uaservercpp/comdamodule)
    set(COMAEMODULE_INCLUDE         ${_BASE_DIR}/uaserver/uaservercpp/comaemodule)
    set(COMHDAMODULE_INCLUDE        ${_BASE_DIR}/uaserver/uaservercpp/comhdamodule)
    set(UACONFIG_INCLUDE            ${_BASE_DIR}/uaserver/uaservercpp/uaconfig)
    set(UATAGFILE_INCLUDE           ${_BASE_DIR}/uaserver/uaservercpp/tagfile)
    set(UAPROCESSINGMODULE_INCLUDE  ${_BASE_DIR}/uaserver/uaservercpp/processingmodule)
    set(UAREDUNDANCY_INCLUDE        ${_BASE_DIR}/uaserver/uaservercpp/uaredundancy)
    set(UAWIDGETS_INCLUDE           ${_BASE_DIR}/uabase/uawidgets)
    set(DATALOGGER_INCLUDE          ${_BASE_DIR}/datalogger
                                    ${_BASE_DIR}/datalogger/${UASDK_PLATFORM}
                                    ${datalogger_BINARY_DIR})

    set(UABASE_LIBRARY              uabase)
    set(UAPKI_LIBRARY               uapki)
    set(UACLIENT_LIBRARY            uaclient)
    set(UAXML_LIBRARY               xmlparser)
    set(UAMODULE_LIBRARY            uamodule)
    set(PUBSUBMODULE_LIBRARY        pubsubmodule)
    set(UACOREMODULE_LIBRARY        coremodule)
    set(UAMODELS_LIBRARY            uamodels)
    set(UACONFIG_LIBRARY            uaconfig)
    set(UATAGFILE_LIBRARY           uatagfile)
    set(UAPROCESSINGMODULE_LIBRARY  processingmodule)
    set(UAREDUNDANCY_LIBRARY        uaredundancy)
    set(UAWIDGETS_LIBRARY           uawidgets)
    set(DATALOGGER_LIBRARY          datalogger)
    set(COMBASE_LIBRARY             combase)
    set(COMDAMODULE_LIBRARY         comdamodule)
    set(COMAEMODULE_LIBRARY         comaemodule)
    set(COMHDAMODULE_LIBRARY        comhdamodule)

    set(UASDK_HAVE_CLIENTLIB OFF)
    if (DEFINED ${UACLIENT_LIBRARY}_SOURCE_DIR)
        set(UASDK_HAVE_CLIENTLIB ON)
    endif ()
    set(UASDK_HAVE_SERVERLIB OFF)
    if (DEFINED ${UACOREMODULE_LIBRARY}_SOURCE_DIR)
        set(UASDK_HAVE_SERVERLIB ON)
    endif ()
else ()
    if (NOT UASDK_BASE_DIR)
        message(FATAL_ERROR "UASDK_BASE_DIR not set")
    endif ()

    set(UABASE_INCLUDE              ${UASDK_BASE_DIR}/include/uabase)
    set(UAPKI_INCLUDE               ${UASDK_BASE_DIR}/include/uapki)
    set(UAXMLPARSER_INCLUDE         ${UASDK_BASE_DIR}/include/xmlparser)
    set(UACOREMODULE_INCLUDE        ${UASDK_BASE_DIR}/include/uaserver)
    set(UAMODULE_INCLUDE            ${UASDK_BASE_DIR}/include/uamodule)
    set(PUBSUBMODULE_INCLUDE        ${UASDK_BASE_DIR}/include/pubsubmodule)
    set(UAMODELS_INCLUDE            ${UASDK_BASE_DIR}/include/uamodels)
    set(UACLIENT_INCLUDE            ${UASDK_BASE_DIR}/include/uaclient)
    set(COMBASE_INCLUDE             ${UASDK_BASE_DIR}/include/combase)
    set(COMBASE_CONFIG_INCLUDE      ${UASDK_BASE_DIR}/include/combase/DCOM_Config)
    set(COMDAMODULE_INCLUDE         ${UASDK_BASE_DIR}/include/comdamodule)
    set(COMAEMODULE_INCLUDE         ${UASDK_BASE_DIR}/include/comaemodule)
    set(COMHDAMODULE_INCLUDE        ${UASDK_BASE_DIR}/include/comhdamodule)
    set(UACONFIG_INCLUDE            ${UASDK_BASE_DIR}/include/uaconfig)
    set(UATAGFILE_INCLUDE           ${UASDK_BASE_DIR}/include/tagfile)
    set(UAPROCESSINGMODULE_INCLUDE  ${UASDK_BASE_DIR}/include/processingmodule)
    set(UAREDUNDANCY_INCLUDE        ${UASDK_BASE_DIR}/include/uaredundancy)
    set(UAWIDGETS_INCLUDE           ${UASDK_BASE_DIR}/include/uawidgets)
    set(DATALOGGER_INCLUDE          ${UASDK_BASE_DIR}/include/datalogger)

    set(UA_LIB_DIR                  ${UASDK_BASE_DIR}/lib)

    set(UABASE_LIBRARY              debug "uabased"             optimized "uabase")
    set(UAPKI_LIBRARY               debug "uapkid"              optimized "uapki")
    set(UACLIENT_LIBRARY            debug "uaclientd"           optimized "uaclient")
    set(UAXML_LIBRARY               debug "xmlparserd"          optimized "xmlparser")
    set(PUBSUBMODULE_LIBRARY        debug "pubsubmoduled"       optimized "pubsubmodule")
    set(UAMODULE_LIBRARY            debug "uamoduled"           optimized "uamodule")
    set(UACOREMODULE_LIBRARY        debug "coremoduled"         optimized "coremodule")
    set(UAMODELS_LIBRARY            debug "uamodelsd"           optimized "uamodels")
    set(UACONFIG_LIBRARY            debug "uaconfigd"           optimized "uaconfig")
    set(UATAGFILE_LIBRARY           debug "uatagfiled"          optimized "uatagfile")
    set(UAPROCESSINGMODULE_LIBRARY  debug "processingmoduled"   optimized "processingmodule")
    set(UAREDUNDANCY_LIBRARY        debug "processingmoduled"   optimized "processingmodule")
    set(UAWIDGETS_LIBRARY           debug "uawidgetsd"          optimized "uawidgets")
    set(DATALOGGER_LIBRARY          debug "dataloggerd"         optimized "datalogger")
    set(COMBASE_LIBRARY             debug "combased"            optimized "combase")
    set(COMDAMODULE_LIBRARY         debug "comdamoduled"        optimized "comdamodule")
    set(COMAEMODULE_LIBRARY         debug "comaemoduled"        optimized "comaemodule")
    set(COMHDAMODULE_LIBRARY        debug "comhdamoduled"        optimized "comhdamodule")

    macro (get_option_from_file _FILENAME _DEFINE_NAME _OPTION_NAME)
        file(STRINGS "${_FILENAME}" TMP_${_OPTION_NAME}
             REGEX "^#[\t ]*define[\t ]+${_DEFINE_NAME}[\t ]+!?OPCUA_CONFIG_(ON|OFF)[\t ]*")
        if (NOT TMP_${_OPTION_NAME})
            message(FATAL_ERROR "Define ${_DEFINE_NAME} missing in config file ${_FILENAME}")
        endif ()
        if (TMP_${_OPTION_NAME} MATCHES "([^!]OPCUA_CONFIG_ON|!OPCUA_CONFIG_OFF)")
            set(${_OPTION_NAME} ON CACHE INTERNAL "")
        else ()
            set(${_OPTION_NAME} OFF CACHE INTERNAL "")
        endif ()
    endmacro ()

    # Use config headers to detect the build configuration
    get_option_from_file("${UABASE_INCLUDE}/uabase.h" SUPPORT_XML_PARSER UASDK_WITH_XMLPARSER)
    # Optional configuration settings, not available for client-only SDK
    if (EXISTS ${DATALOGGER_INCLUDE})
        get_option_from_file("${DATALOGGER_INCLUDE}/uaserver_datalogger_config.h" HAVE_DATA_LOGGER_FILE_BACKEND BUILD_DATA_LOGGER_FILE_BACKEND)
        get_option_from_file("${DATALOGGER_INCLUDE}/uaserver_datalogger_config.h" HAVE_DATA_LOGGER_SQLITE_BACKEND BUILD_DATA_LOGGER_SQLITE_BACKEND)
    endif ()

    find_library(UASDK_HAVE_CLIENTLIB NAMES uaclient uaclient${CMAKE_DEBUG_POSTFIX} PATHS "${UA_LIB_DIR}")
    mark_as_advanced(UASDK_HAVE_CLIENTLIB)
    find_library(UASDK_HAVE_SERVERLIB NAMES coremodule coremodule${CMAKE_DEBUG_POSTFIX} PATHS "${UA_LIB_DIR}")
    mark_as_advanced(UASDK_HAVE_SERVERLIB)
endif ()

# set SYSTEM_LIBS depending on configuration
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    list(APPEND SYSTEM_LIBS ws2_32 mpr)
    if (UASTACK_WITH_PKI_WIN32)
        list(APPEND SYSTEM_LIBS crypt32)
    endif ()
endif ()

# deprecated compat variables
