cmake_minimum_required(VERSION 3.4...3.18)

# TODO: Find a way to make this automatic like setuptools_scm
project(rssdk VERSION 3.20.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_DEBUG_POSTFIX "d")

include(GNUInstallDirs)

option(BUILD_TESTS "Build all test" OFF)
option(BUILD_UTILITIES "Build command line control utilities" OFF)
option(INSTALL_UTILITIES "Installs command line control utilities" OFF)
option(INSTALL_SDK "Install SDK files" ON)
option(BUILD_SHARED_LIBS "Build as shared library" ON)
option(BUILD_PYTHON_BINDINGS "Build Python bindings" OFF)
option(INSTALL_PYTHON_BINDINGS "Install Python bindings to current interpreter" OFF)

if (NOT BUILD_UTILITIES AND INSTALL_UTILITIES)
    message(WARNING 
        " INSTALL_UTILITES is ON but BUILD_UTILITIES is OFF.\n"
        " Not building or installing utilities..."
    )
    set(INSTALL_UTILITIES OFF)
endif()

# Detect target architecture so we can 
# copy / install the correct driver dll file.
include(TargetArch)
target_architecture(TARGET_ARCH)

message(STATUS "Building for ${TARGET_ARCH}")

if ("${TARGET_ARCH}" STREQUAL "x86_64")
    set(DRIVER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/driver/x64/drv.dll)
elseif("${TARGET_ARCH}" STREQUAL "i386")
    set(DRIVER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/driver/x32/drv.dll)
endif()

if (WIN32)
    message(STATUS "Using ${DRIVER_FILE}")
endif()

if (BUILD_TESTS)
    enable_testing()
endif()

set(_pic OFF)
if (BUILD_SHARED_LIBS OR BUILD_PYTHON_BINDINGS)
    set(_pic ON)
endif()

message(STATUS "Building libraries with position independent code ${_pic}")


add_subdirectory(src)




set_target_properties(rserrors rsdio rspoe PROPERTIES 
    COMPILE_DEFINITIONS "RSSDK_VERSION_STRING=\"${PROJECT_VERSION}\""
    VERSION ${rssdk_VERSION}
    SOVERSION ${rssdk_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ${_pic}
)

if(MSVC)
    target_compile_definitions(rsdio PUBLIC -D_CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(rspoe PUBLIC -D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

if (BUILD_PYTHON_BINDINGS)
    find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
    find_package(pybind11 CONFIG REQUIRED)
    
    # Add the python bindings folder
    add_subdirectory(extras/python)
endif()



#*********#
# Install #
#*********#
if (INSTALL_SDK)
    if (WIN32)
        install(FILES ${DRIVER_FILE} TYPE BIN)
    endif()
endif()

if(INSTALL_UTILITIES)
    if(MSVC)
        set(CMAKE_INSTALL_SBINDIR bin)
    endif(MSVC)
endif(INSTALL_UTILITIES)
