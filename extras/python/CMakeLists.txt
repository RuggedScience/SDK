# Define the python module
# python_add_library is a wrapper provided by FindPython (or pybind11_add_module)
pybind11_add_module(rssdk_python MODULE rssdk.cpp)

# Link your internal libraries
target_link_libraries(rssdk_python PRIVATE rsdio rspoe rserrors)

# Rename output to 'rssdk' so in Python you do "import rssdk"
set_target_properties(rssdk_python PROPERTIES OUTPUT_NAME "rssdk")

# INSTALLATION
# scikit-build-core looks for install rules to decide what goes into the wheel.
install(TARGETS rssdk_python DESTINATION .)
