
cmake_minimum_required(VERSION 3.1)

project(rssdk VERSION 3.0.3)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(GNUInstallDirs)
option(INSTALL_UTILITIES "Installs utilities" ON)
option(INSTALL_XML "Installs XML files" ON)
option(BUILD_SHARED_LIBS "Build as shared library" ON)

add_subdirectory(error)
add_subdirectory(dio)
add_subdirectory(poe)

add_executable(rsdioctl rsdioctl.cpp)
add_executable(rspoectl rspoectl.cpp)

target_link_libraries(rsdioctl rsdio)
target_link_libraries(rspoectl rspoe)

SET_TARGET_PROPERTIES(rsdioctl rspoectl
    PROPERTIES INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/"
)

get_target_property(rserrors_SOURCES rserrors SOURCES)

get_target_property(rsdio_SOURCES rsdio SOURCES)
add_executable(rsdioimpl_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_rsdioimpl.cpp
    ${rserrors_SOURCES}
    ${rsdio_SOURCES}
)
target_compile_definitions(rsdioimpl_test PUBLIC NO_EXPORT)

get_target_property(rspoe_SOURCES rspoe SOURCES)
add_executable(rspoeimpl_test
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_rspoeimpl.cpp
    ${rserrors_SOURCES}
    ${rspoe_SOURCES}
)
target_compile_definitions(rspoeimpl_test PUBLIC NO_EXPORT)

add_executable(rsdio_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_rsdio.cpp
)
target_link_libraries(rsdio_test PRIVATE rsdio)

add_executable(rspoe_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_rspoe.cpp
)
target_link_libraries(rspoe_test PRIVATE rspoe)

add_executable(rserrors_test
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_rserrors.cpp
)
target_link_libraries(rserrors_test PRIVATE rsdio rspoe)

enable_testing()

add_test(
    NAME rserrors_test
    COMMAND rserrors_test
)

add_test(
    NAME rsdioimpl_test
    COMMAND rsdioimpl_test
)

add_test(
    NAME rspoeimpl_test
    COMMAND rspoeimpl_test
)

add_test(
    NAME rsdio_test 
    COMMAND rsdio_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

add_test(
    NAME rspoe_test 
    COMMAND rspoe_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND UNIX)
    set (CMAKE_INSTALL_SYSCONFDIR /etc)
    set (CMAKE_INSTALL_FULL_SYSCONFDIR /etc)
endif()

if(INSTALL_UTILITIES)
    if(MSVC)
        set(CMAKE_INSTALL_SBINDIR bin)
    endif(MSVC)

    install(TARGETS rsdioctl DESTINATION ${CMAKE_INSTALL_SBINDIR})
    install(TARGETS rspoectl DESTINATION ${CMAKE_INSTALL_SBINDIR})
endif(INSTALL_UTILITIES)

if (INSTALL_XML)
    install(DIRECTORY xml/ DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${CMAKE_PROJECT_NAME})
endif(INSTALL_XML)
