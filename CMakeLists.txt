
cmake_minimum_required(VERSION 3.1)

project(rssdk VERSION 3.0.3)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(dio)
add_subdirectory(poe)

include(GNUInstallDirs)
option(BUILD_PYTHON_BINDINGS "Build Python bindings using cython" OFF)
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

add_custom_target(xml ALL)
add_custom_command(TARGET xml POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${PROJECT_SOURCE_DIR}/xml/ ${CMAKE_CURRENT_BINARY_DIR}/xml/)

# Python bindings
if(BUILD_PYTHON_BINDINGS)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)

  # Prepare Python's setup.cfg
  file(GENERATE
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/extras/python-bindings/setup.py
    INPUT ${CMAKE_CURRENT_SOURCE_DIR}/extras/python-bindings/setup.py.in
)

  # Create version file for python package
  file(GENERATE
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/extras/python-bindings/version.txt
    CONTENT "${CMAKE_PROJECT_VERSION}"
  )

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/python.stamp
    COMMAND ${Python3_EXECUTABLE} -m build -o ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/python.stamp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/extras/python-bindings
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/extras/python-bindings/setup.py
      ${CMAKE_CURRENT_SOURCE_DIR}/extras/python-bindings/version.txt
      ${CMAKE_CURRENT_SOURCE_DIR}/extras/python-bindings/src/rssdk/__init__.py
      ${CMAKE_CURRENT_SOURCE_DIR}/extras/python-bindings/src/rssdk/rsdio.pyx
      ${CMAKE_CURRENT_SOURCE_DIR}/extras/python-bindings/src/rssdk/rspoe.pyx
      rsdio
      rspoe
    COMMENT "Building python bindings"
  )
  add_custom_target(rssdk-python-bindings ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/python.stamp)
  SET_TARGET_PROPERTIES(rssdk-python-bindings PROPERTIES ADDITIONAL_CLEAN_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/extras/python-bindings/setup.cfg;${CMAKE_CURRENT_SOURCE_DIR}/extras/python-bindings/version.txt;${CMAKE_CURRENT_SOURCE_DIR}/extras/python-bindings/src/rssdk.egg-info")
endif()

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
