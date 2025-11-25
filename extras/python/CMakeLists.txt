# Define the python module
# python_add_library is a wrapper provided by FindPython (or pybind11_add_module)
pybind11_add_module(rssdk_python MODULE rssdk.cpp)

# Link your internal libraries
target_link_libraries(rssdk_python PRIVATE rsdio rspoe rserrors)

# Rename output to 'rssdk' so in Python you do "import rssdk"
set_target_properties(rssdk_python PROPERTIES OUTPUT_NAME "rssdk")

set(RSSDK_PKG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/rssdk")

find_package(Python COMPONENTS Interpreter REQUIRED)

add_custom_command(TARGET rssdk_python POST_BUILD
    COMMAND ${Python_EXECUTABLE} -m pybind11_stubgen rssdk --output-dir "${RSSDK_PKG_DIR}"
    COMMENT "Generating type stubs for rssdk"
)

# --- INSTALLATION ---
# scikit-build-core looks for install rules to decide what goes into the wheel.
install(TARGETS rssdk_python DESTINATION rssdk)

# Install the Generated Stub
# The stub will be named rssdk.pyi (or rssdk/__init__.pyi if it's a package)
install(FILES "${RSSDK_PKG_DIR}/rssdk.pyi" DESTINATION rssdk)

