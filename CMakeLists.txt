
cmake_minimum_required(VERSION 3.4...3.18)

project(rssdk)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(GNUInstallDirs)
option(BUILD_TESTS "Build all test" OFF)
option(BUILD_UTILITIES "Build command line control utilities" OFF)
option(INSTALL_UTILITIES "Installs command line control utilities" OFF)
option(INSTALL_XML "Installs XML files" ON)
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

add_subdirectory(error)
add_subdirectory(dio)
add_subdirectory(poe)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()

if (BUILD_PYTHON_BINDINGS)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extras/python)
endif()

add_executable(rsdioctl rsdioctl.cpp)
add_executable(rspoectl rspoectl.cpp)

target_link_libraries(rsdioctl rsdio)
target_link_libraries(rspoectl rspoe)

SET_TARGET_PROPERTIES(rsdioctl rspoectl
    PROPERTIES INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/"
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
