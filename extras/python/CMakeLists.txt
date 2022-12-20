# Find Python3
find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)
find_package(pybind11 REQUIRED)

set(PYBIND11_CPP_STANDARD -std=c++11)

pybind11_add_module(rssdkpy MODULE rssdk.cpp)

target_link_libraries(rssdkpy PRIVATE rserrors rsdio rspoe)

set_target_properties(rssdkpy PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/rssdk
    OUTPUT_NAME "rssdk"
)

# Handle where to install the resulting Python package
if(CALL_FROM_SETUP_PY)
    # The CMakeExtension will set CMAKE_INSTALL_PREFIX to the root
    # of the resulting wheel archive
    set(RSSDKPY_INSTDIR ${CMAKE_INSTALL_PREFIX})
else()
    # The Python package is installed directly in the folder of the
    # detected interpreter (system, user, or virtualenv)
    set(RSSDKPY_INSTDIR ${Python3_SITELIB})
    if (BUILD_SHARED_LIBS)
        message(WARNING
            " Building the Python bindings with shared libs can results in an\n"
            " ImportError when importing the module and is not recommended"
        )
    endif()
endif()

message(STATUS "Installing rssdkpy in ${RSSDKPY_INSTDIR}")

# Setup installation path
install(TARGETS rssdkpy COMPONENT python DESTINATION "${RSSDKPY_INSTDIR}/rssdk")

# Create the Python package in the build tree for testing purposes
set(RSSDKPY_BUILDDIR "${CMAKE_BINARY_DIR}/rssdk")
set_target_properties(
  rssdkpy PROPERTIES
  OUTPUT_NAME rssdk
  LIBRARY_OUTPUT_DIRECTORY "${RSSDKPY_BUILDDIR}")

# Create the __init__.py file
file(
  GENERATE
  OUTPUT "${RSSDKPY_BUILDDIR}/__init__.py"
  CONTENT "from rssdk.rssdk import *\n")

# Install the __init__.py file
install(
  FILES "${RSSDKPY_BUILDDIR}/__init__.py"
  DESTINATION ${RSSDKPY_INSTDIR}/rssdk)

# See https://packaging.python.org/specifications/recording-installed-packages/
# and https://packaging.python.org/en/latest/specifications/core-metadata/#core-metadata
if (NOT CALL_FROM_SETUP_PY)
  execute_process(
    COMMAND ${Python3_EXECUTABLE} version.py
    WORKING_DIRECTORY ${rssdk_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_VERSION
  )

  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/METADATA "")
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/METADATA "Metadata-Version: 2.1\n")
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/METADATA "Name: ${PROJECT_NAME}\n")
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/METADATA "Version: ${PROJECT_VERSION}\n")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/INSTALLER "cmake\n")
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/METADATA" "${CMAKE_CURRENT_BINARY_DIR}/INSTALLER"
    DESTINATION ${RSSDKPY_INSTDIR}/rssdk-${PROJECT_VERSION}.dist-info)
endif()
