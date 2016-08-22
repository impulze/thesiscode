project(uaserver)
cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(UaOpenSSL)
find_package(UaLibXml2)

set(UASDK_BASE_DIR "/home/impulze/install/build/sdk")

set(ADAPTER_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/../adapter/include" "${CMAKE_CURRENT_SOURCE_DIR}/../wrapper/include")
set(ADAPTER_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../adapter/lib" "${CMAKE_CURRENT_SOURCE_DIR}/../wrapper/lib")
set(ADAPTER_LIBRARIES "adapter" "wrapper" "pthread")

set(MODELS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/../models")
file(GLOB MODELS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../models/gen_models/*.cpp")
file(GLOB MODELS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/../models/gen_models/*.h")

include(MessageUtils)
include(ConfigureCompiler)
include(ConfigureUaStack)
include(ConfigureCppSdk)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH ${UA_LIB_DIR})
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

display_project_header()

####################################### Different Configurations ########################################

if (NOT UASTACK_SUPPORT_ENC_OBJ_EXTENSIONS)
    message(FATAL_ERROR "It is not allowed to to build the application without UASTACK_SUPPORT_ENC_OBJ_EXTENSIONS")
endif ()

# Build with shared stack
if (BUILD_SHARED_STACK OR BUILD_SHARED_LIBS)
    add_definitions(-D_UA_STACK_USE_DLL)
endif ()

# configure Dll/Lib
if (BUILD_SHARED_LIBS)
    add_definitions(-D_UA_BASE_USE_DLL)
    if (UASDK_WITH_XMLPARSER)
        add_definitions(-D_XML_PARSER_USE_DLL)
    endif ()
    add_definitions(-D_UA_PKI_USE_DLL)
    add_definitions(-D_SERVER_CORE_USE_DLL)
    add_definitions(-D_UA_MODULE_USE_DLL)
    add_definitions(-D_UAMODELS_USE_DLL)
endif ()

if (UASDK_WITH_XMLPARSER)
    set(CONFIG_FILE ServerConfig.xml)
else ()
    set(CONFIG_FILE ServerConfig.ini)
endif ()

file(GLOB PROJECT_H "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
set(PROJECT_H ${PROJECT_H} "${CMAKE_CURRENT_SOURCE_DIR}/utilities/shutdown.h" "${CMAKE_CURRENT_SOURCE_DIR}/utilities/opcserver.h")

####################################### Include Paths ########################################

include_directories(${UASTACK_INCLUDE})
include_directories(${UABASE_INCLUDE})
include_directories(${UAPKI_INCLUDE})
include_directories(${UAXMLPARSER_INCLUDE})
include_directories(${UACOREMODULE_INCLUDE})
include_directories(${UAMODULE_INCLUDE})
include_directories(${UAMODELS_INCLUDE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utilities)
include_directories(${ADAPTER_INCLUDE})
include_directories(${MODELS_INCLUDE})

# LINKER PATHS
if (NOT TARGET uastack)
    link_directories(${UA_LIB_DIR})
endif ()
link_directories(${ADAPTER_LIB_DIR})

####################################### Create executable ###################################

add_executable(${PROJECT_NAME} ${PROJECT_H}
    servermain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/opcserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/shutdown.cpp
   $<TARGET_OBJECTS:models>
)

####################################### Link Library ########################################

target_link_libraries(${PROJECT_NAME}
                      ${UAMODELS_LIBRARY}
                      ${UAMODULE_LIBRARY}
                      ${UACOREMODULE_LIBRARY}
                      ${UAPKI_LIBRARY}
                      ${UABASE_LIBRARY}
                      ${UASTACK_LIBRARY}
                      ${SYSTEM_LIBS}
                      ${ADAPTER_LIBRARIES})

if (UASDK_WITH_XMLPARSER)
    target_link_libraries(${PROJECT_NAME} ${UAXML_LIBRARY} ${LIBXML2_LIBRARIES})
endif ()
if (UASTACK_WITH_OPENSSL)
    target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
endif ()

add_library(models OBJECT ${MODELS_SOURCES})
include_directories(${MODELS_INCLUDE})
set_target_properties(models PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})


include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()
if(COMPILER_SUPPORTS_CXX11 OR COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-literal-suffix")
endif()
