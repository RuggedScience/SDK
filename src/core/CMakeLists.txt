add_library(core INTERFACE)

target_include_directories(core INTERFACE
    # This path is relative to the directory where my_headers is defined
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    # This path is where headers will be installed for external users
    $<INSTALL_INTERFACE:include> 
)