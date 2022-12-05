
cmake_minimum_required(VERSION 3.1)

project(rssdk VERSION 3.0.3)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_subdirectory(error)
add_subdirectory(dio)
add_subdirectory(poe)


include(GNUInstallDirs)
option(INSTALL_UTILITIES "Installs utilities" ON)
option(INSTALL_XML "Installs XML files" ON)

add_executable(rsdioctl rsdioctl.cpp)
add_executable(rspoectl rspoectl.cpp)

target_include_directories(rsdioctl PRIVATE "${PROJECT_BINARY_DIR}/dio")
target_include_directories(rspoectl PRIVATE "${PROJECT_BINARY_DIR}/poe")

target_link_libraries(rsdioctl rsdio)
target_link_libraries(rspoectl rspoe)

SET_TARGET_PROPERTIES(rsdioctl rspoectl
    PROPERTIES INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")

add_executable(rsdio_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_rsdio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/argparse.cpp
)
target_link_libraries(rsdio_test rsdio rspoe)
target_include_directories(rsdio_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/utils)

add_executable(rspoe_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_rspoe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/argparse.cpp
)
target_link_libraries(rspoe_test rspoe rsdio)
target_include_directories(rspoe_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/utils)

# enable testing functionality
enable_testing()

add_test(NAME rsdio_test COMMAND $<TARGET_FILE:rsdio_test> WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_custom_target(xml ALL)
add_custom_command(TARGET xml POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${PROJECT_SOURCE_DIR}/xml/ ${CMAKE_CURRENT_BINARY_DIR}/xml/)

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

install(FILES utils/rssdk_errors.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME})
