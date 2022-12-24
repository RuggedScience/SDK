cmake_minimum_required(VERSION 3.12...3.18)

# Our setup.py sets this to ensure pybind11 uses the right interpreter
# If it isn't set, let's default to cmake's FindPython3 module.
if (NOT PYTHON_EXECUTABLE)
  find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
  # If we find Python for pybind11, it won't set these and we need them.
  set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
  set(PYTHON_SITE_PACKAGES ${Python3_SITELIB})
endif()

# Try to find pybind11 using provided CMAKE_PREFIX_PATH
find_package(pybind11 QUIET)

# If it wasn't found try to get the cmake path
# and add it to the CMAKE_PREFIX_PATH and try again.
if (NOT pybind11_FOUND)
  message(DEBUG "pybind11 not found. Attempting to use Python package")
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -m pybind11 --cmake
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
  # Add RPATH to the python binding library file.
  # This allows shared lib installs on Linux to find the libs.
  SET(CMAKE_SKIP_BUILD_RPATH FALSE)
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()

set(CMAKE_DEBUG_POSTFIX "")
set(PYBIND11_CPP_STANDARD -std=c++11)

pybind11_add_module(rssdkpy MODULE rssdk.cpp)
target_link_libraries(rssdkpy PRIVATE rserrors rsdio rspoe)
set_target_properties(rssdkpy PROPERTIES OUTPUT_NAME "rssdk")

# We need the libs in the same directory on Windows
# when building as a shared library.
if (BUILD_SHARED_LIBS AND WIN32)
  add_custom_command(TARGET rssdkpy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:rserrors>" "$<TARGET_FILE_DIR:rssdkpy>"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:rsdio>" "$<TARGET_FILE_DIR:rssdkpy>"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:rspoe>" "$<TARGET_FILE_DIR:rssdkpy>"
    COMMENT "Copying shared libs to Python build folder"
  )
endif()

#************#
# Stub Files #
# ***********#
find_program(_STUBGEN_EXISTS stubgen)
if (_STUBGEN_EXISTS)
  add_custom_command(TARGET rssdkpy POST_BUILD
    COMMAND stubgen -v -p rssdk -o .
    COMMAND ${CMAKE_COMMAND} -E touch py.typed
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:rssdkpy>"
    COMMENT "Generating Python stub files in Python build folder"
  )
else()
  message(WARNING 
    " stubgen not found. Stub files will not be generated.\n"
    " To generate stub files install mypy with `python -m pip install mypy`."
  )
endif()

if (WIN32)
  add_custom_command(TARGET rssdkpy
    COMMAND ${CMAKE_COMMAND} -E copy ${DRIVER_FILE} $<TARGET_FILE_DIR:rssdkpy>
  )
endif()

#*********#
# Install #
#*********#
if (INSTALL_PYTHON_BINDINGS)
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "from setuptools_scm import get_version; print(get_version());"
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

  message(WARNING
  " This will install the rssdk package directly into your current Python environment.\n"
  " In order to uninstall it you will need to delete the following two folders manually.\n"
  " \n"
  " \t\t${PYTHON_SITE_PACKAGES}/rssdk\n"
  " \t\t${PYTHON_SITE_PACKAGES}/rssdk-${RSSDKPY_VERSION}.dist-info"
  )

  set(RSSDKPY_INSTDIR ${PYTHON_SITE_PACKAGES}/rssdk)
  set(RSSDKPY_METADIR ${PYTHON_SITE_PACKAGES}/rssdk-${RSSDKPY_VERSION}.dist-info)

  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/METADATA "")
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/METADATA "Metadata-Version: 2.1\n")
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/METADATA "Name: ${PROJECT_NAME}\n")
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/METADATA "Version: ${RSSDKPY_VERSION}\n")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/INSTALLER "cmake\n")

  install(TARGETS rssdkpy COMPONENT python DESTINATION "${RSSDKPY_INSTDIR}")
  # Install the shared libs with the package
  # on Windows to prevent import error.
  # On Linux we use RPATH
  if (BUILD_SHARED_LIBS AND WIN32)
    install(TARGETS rserrors rsdio rspoe COMPONENT python DESTINATION "${RSSDKPY_INSTDIR}")
  endif()

  install(FILES 
      "${CMAKE_CURRENT_SOURCE_DIR}/rssdk/__init__.py"
      "$<TARGET_FILE_DIR:rssdkpy>/rssdk.pyi"
      "$<TARGET_FILE_DIR:rssdkpy>/py.typed"
    DESTINATION "${RSSDKPY_INSTDIR}"
  )
  
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/METADATA" "${CMAKE_CURRENT_BINARY_DIR}/INSTALLER"
    DESTINATION ${RSSDKPY_METADIR}
  )

  if (WIN32)
    install(FILES ${DRIVER_FILE} DESTINATION ${RSSDKPY_INSTDIR})
  endif()

  # Install the libraries with the Python package if 
  # they are built as shared libs and not being installed.
  if (BUILD_SHARED_LIBS AND NOT INSTALL_SDK)
    install(
      TARGETS rserrors rsdio rspoe
      RUNTIME DESTINATION ${RSSDKPY_INSTDIR}
    )
  endif()
endif()
