cmake_minimum_required(VERSION 3.4...3.18)

# TODO: Find a way to make this automatic like setuptools_scm
project(rssdk VERSION 3.19.0)

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

add_library(rserrors 
    ${CMAKE_CURRENT_SOURCE_DIR}/error/src/rserrors.cpp
)
target_include_directories(
    rserrors PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/error/include>"
)
target_compile_definitions(
    rserrors PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:RSERRORS_STATIC_DEFINE>"
)

add_library(rsdio
    ${CMAKE_CURRENT_SOURCE_DIR}/dio/src/rsdioimpl.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/dio/src/controllers/ite8783.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/dio/src/controllers/ite8786.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/tinyxml2.cpp
)
target_link_libraries(rsdio PUBLIC rserrors)
target_include_directories(
    rsdio PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dio/include>"
)
target_compile_definitions(
    rsdio PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:RSDIO_STATIC_DEFINE>"
)

add_library(rspoe
    ${CMAKE_CURRENT_SOURCE_DIR}/poe/src/rspoeimpl.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/poe/src/controllers/pd69104.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/poe/src/controllers/pd69200.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/poe/src/controllers/ltc4266.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/tinyxml2.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/i801_smbus.cpp
)
target_link_libraries(rspoe PUBLIC rserrors)
target_include_directories(
    rspoe PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/poe/include>"
)
target_compile_definitions(
    rspoe PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:RSPOE_STATIC_DEFINE>"
)

set(_pic OFF)
if (BUILD_SHARED_LIBS OR BUILD_PYTHON_BINDINGS)
    set(_pic ON)
endif()

message(STATUS "Building libraries with position independent code ${_pic}")

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

include(GenerateExportHeader)
generate_export_header(rserrors EXPORT_FILE_NAME include/rserrors_export.h)
generate_export_header(rsdio EXPORT_FILE_NAME include/rsdio_export.h)
generate_export_header(rspoe EXPORT_FILE_NAME include/rspoe_export.h)

target_include_directories(
    rserrors PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
)
target_include_directories(
    rsdio PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
)
target_include_directories(
    rspoe PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
)

#*********#
# Testing #
#*********#

if (BUILD_TESTS)
    enable_testing()
    get_target_property(rserrors_SOURCES rserrors SOURCES)
    get_target_property(rsdio_SOURCES rsdio SOURCES)
    get_target_property(rspoe_SOURCES rspoe SOURCES)

    add_executable(rsdioimpl_test 
        tests/test_rsdioimpl.cpp
        ${rserrors_SOURCES}
        ${rsdio_SOURCES}
    )
    target_compile_definitions(rsdioimpl_test PUBLIC NO_EXPORT)

    get_target_property(rspoe_SOURCES rspoe SOURCES)
    add_executable(rspoeimpl_test
        tests/test_rspoeimpl.cpp
        ${rserrors_SOURCES}
        ${rspoe_SOURCES}
    )
    target_compile_definitions(rspoeimpl_test PUBLIC NO_EXPORT)

    add_executable(rsdio_test tests/test_rsdio.cpp)
    target_link_libraries(rsdio_test PRIVATE rsdio)

    add_executable(rspoe_test tests/test_rspoe.cpp)
    target_link_libraries(rspoe_test PRIVATE rspoe)

    add_executable(rserrors_test tests/test_rserrors.cpp)
    target_link_libraries(rserrors_test PRIVATE rsdio rspoe)

    add_test(NAME rserrors_test COMMAND rserrors_test)
    add_test(NAME rsdioimpl_test COMMAND rsdioimpl_test)

    add_test(NAME rspoeimpl_test COMMAND rspoeimpl_test) 

    add_test(NAME rsdio_test COMMAND rsdio_test
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    add_test(NAME rspoe_test COMMAND rspoe_test
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
endif()

if (BUILD_PYTHON_BINDINGS)
    add_subdirectory(extras/python)
endif()

if (BUILD_UTILITIES)
    add_executable(rsdioctl rsdioctl.cpp)
    add_executable(rspoectl rspoectl.cpp)

    target_link_libraries(rsdioctl rsdio)
    target_link_libraries(rspoectl rspoe)

    set_target_properties(rsdioctl rspoectl
        PROPERTIES INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/"
    )
endif()

#*********#
# Install #
#*********#

if (INSTALL_SDK)
    
    include(GNUInstallDirs)

    if (NOT DEFINED rssdk_INSTALL_CMAKEDIR)
        set(rssdk_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/rssdk"
            CACHE STRING "Path to rssdk CMake files")
    endif ()

    install(TARGETS rserrors rsdio rspoe EXPORT rssdk_Targets
        RUNTIME COMPONENT rssdk_Runtime
        LIBRARY COMPONENT rssdk_Runtime
        NAMELINK_COMPONENT rssdk_Development
        ARCHIVE COMPONENT rssdk_Development
        INCLUDES COMPONENT rssdk_Development
    )
    
    install(DIRECTORY 
        "${rssdk_SOURCE_DIR}/error/include/"
        "${rssdk_SOURCE_DIR}/dio/include/"
        "${rssdk_SOURCE_DIR}/poe/include/"
        "${CMAKE_CURRENT_BINARY_DIR}/include/"
        TYPE INCLUDE
    )

    if (WIN32)
        install(FILES ${DRIVER_FILE} TYPE BIN)
    endif()
endif()

if(INSTALL_UTILITIES)
    if(MSVC)
        set(CMAKE_INSTALL_SBINDIR bin)
    endif(MSVC)

    install(TARGETS rsdioctl rspoectl)
endif(INSTALL_UTILITIES)
