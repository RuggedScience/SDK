# Find Python3
find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)

# Try to find pybind11 using provided CMAKE_PREFIX_PATH
find_package(pybind11 QUIET)

# If it wasn't found try to get the cmake path
# and add it to the CMAKE_PREFIX_PATH and try again.
if (NOT pybind11_FOUND)
  message(DEBUG "pybind11 not found. Attempting to use Python package")
  execute_process(
    COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmake
    OUTPUT_VARIABLE PYBIND11_CMAKE
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  list(APPEND CMAKE_PREFIX_PATH ${PYBIND11_CMAKE})
  find_package(pybind11 QUIET)

  if (NOT pybind11_FOUND)
    message(FATAL_ERROR
      " Unable to find pybind11 package.\n"
      " Either pass it's cmake path using -DCMAKE_PREFIX_PATH=/path/to/pybind11\n"
      " or install the Python package using `python -m pip install pybind11`"
    )
  endif()
endif()

message(STATUS
    "Found pybind11: ${pybind11_INCLUDE_DIR} (found version \"${pybind11_VERSION}${pybind11_VERSION_TYPE}\")"
)

if (BUILD_SHARED_LIBS)
  message(WARNING
    " Building the Python bindings with shared libs can results in an\n"
    " ImportError when importing the module and is not recommended"
  )
endif()

set(PYBIND11_CPP_STANDARD -std=c++11)
# Add RPATH to the python binding library file.
# This allows shared lib installs on Linux to find the libs.
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_DEBUG_POSTFIX "")

pybind11_add_module(rssdkpy MODULE rssdk.cpp)

target_link_libraries(rssdkpy PRIVATE rserrors rsdio rspoe)

set_target_properties(rssdkpy PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/rssdk
    OUTPUT_NAME "rssdk"
)

set(RSSDKPY_BUILDDIR "${CMAKE_BINARY_DIR}/rssdk")
set_target_properties(
  rssdkpy PROPERTIES
  OUTPUT_NAME rssdk
  LIBRARY_OUTPUT_DIRECTORY "${RSSDKPY_BUILDDIR}"
)

if (MSVC)
  set_target_properties(
    rssdkpy PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${RSSDKPY_BUILDDIR}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${RSSDKPY_BUILDDIR}"
  )
endif()

# Create the __init__.py file
file(
  GENERATE
  OUTPUT "${RSSDKPY_BUILDDIR}/__init__.py"
  CONTENT "from rssdk.rssdk import *\n"
)

# If we didn't build this from Python's build system
# We need to generate the package metadata info.
if (NOT CALL_FROM_SETUP_PY)
  execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "from setuptools_scm import get_version; print(get_version());"
    WORKING_DIRECTORY ${rssdk_SOURCE_DIR}
    OUTPUT_VARIABLE RSSDKPY_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if ("${RSSDKPY_VERSION}" STREQUAL "")
    message(FATAL_ERROR
      " Python package setuptools_scm is required for versioning.\n"
      " To install it run `python -m pip install setuptools_scm`"
    )
  endif()

  message(STATUS "Using python package version ${RSSDKPY_VERSION}")

  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/METADATA "")
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/METADATA "Metadata-Version: 2.1\n")
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/METADATA "Name: ${PROJECT_NAME}\n")
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/METADATA "Version: ${RSSDKPY_VERSION}\n")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/INSTALLER "cmake\n")
endif()

#************#
# Stub Files #
# ***********#
add_custom_command(TARGET rssdkpy POST_BUILD
  COMMAND ${Python3_EXECUTABLE} -c "import os; print(os.listdir('.'));"
  COMMAND stubgen -v -p rssdk -o .
  COMMAND ${CMAKE_COMMAND} -E touch py.typed
  WORKING_DIRECTORY ${RSSDKPY_BUILDDIR}
  COMMENT "Generating Python stub files in ${RSSDKPY_BUILDDIR}"
)


#*********#
# Install #
# ********#

if (CALL_FROM_SETUP_PY)
  # This will be set correctly when running
  # from Python's build system.
  set(RSSDKPY_INSTDIR ${CMAKE_INSTALL_PREFIX})
elseif (INSTALL_PYTHON_BINDINGS)
  # The Python package is installed directly in the folder of the
  # detected interpreter (system, user, or virtualenv)
  set(RSSDKPY_INSTDIR ${Python3_SITELIB}/rssdk)
  set(RSSDKPY_METADIR ${Python3_SITELIB}/rssdk-${RSSDKPY_VERSION}.dist-info)
endif()

if (RSSDKPY_INSTDIR)
  message(STATUS "Installing rssdk Python package in ${RSSDKPY_INSTDIR}")
  # Setup installation path
  install(TARGETS rssdkpy COMPONENT python DESTINATION "${RSSDKPY_INSTDIR}")
  
# Install the __init__.py file
  install(
    FILES 
      "${RSSDKPY_BUILDDIR}/__init__.py" 
      "${RSSDKPY_BUILDDIR}/rssdk.pyi"
      "${RSSDKPY_BUILDDIR}/py.typed"
    DESTINATION ${RSSDKPY_INSTDIR}
  )
  if (WIN32)
    install(
      FILES "${rssdk_SOURCE_DIR}/driver/drv.dll"
      DESTINATION ${RSSDKPY_INSTDIR}
    )
    if (BUILD_SHARED_LIBS)
      install(
        TARGETS rserrors rsdio rspoe
        RUNTIME DESTINATION ${RSSDKPY_INSTDIR}
      )
    endif()
  endif()
endif()

if (RSSDKPY_METADIR)
  message(STATUS "Installing rssdk Python package metadata in ${RSSDKPY_METADIR}")
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/METADATA" "${CMAKE_CURRENT_BINARY_DIR}/INSTALLER"
    DESTINATION ${RSSDKPY_METADIR}
  )
endif()
