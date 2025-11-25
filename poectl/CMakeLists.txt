add_executable(rspoectl 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rspoectl.cpp
)

target_link_libraries(rspoectl rspoe)

set_target_properties(rspoectl PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib"
    INSTALL_RPATH_USE_LINK_PATH FALSE # Don't bake in the build directory paths
    BUILD_WITH_INSTALL_RPATH FALSE    # Use build paths for testing, change on install
)

if(INSTALL_UTILITIES)
    install(TARGETS rspoectl)
endif()