project(rssdk)

cmake_minimum_required(VERSION 2.8)

if (CMAKE_VERSION VERSION_LESS "3.1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    set(CMAKE_CXX_STANDARD 11)
endif()

add_subdirectory(dio)
add_subdirectory(poe)

include(GNUInstallDirs)
option(INSTALL_UTILITIES "Installs utilities" ON)

add_executable(rsdioctl rsdioctl.cpp)
add_executable(rspoectl rspoectl.cpp)

target_include_directories(rsdioctl PRIVATE "${PROJECT_BINARY_DIR}/dio")
target_include_directories(rspoectl PRIVATE "${PROJECT_BINARY_DIR}/poe")

target_link_libraries(rsdioctl rsdio)
target_link_libraries(rspoectl rspoe)

SET_TARGET_PROPERTIES(rsdioctl rspoectl
    PROPERTIES INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")


if(INSTALL_UTILITIES)
    if(MSVC)
        set(CMAKE_INSTALL_BINDIR bin)
    endif(MSVC)

    install(TARGETS rsdioctl DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(TARGETS rspoectl DESTINATION ${CMAKE_INSTALL_BINDIR})
endif(INSTALL_UTILITIES)
