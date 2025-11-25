add_executable(rsdioctl 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rsdioctl.cpp
)

target_link_libraries(rsdioctl rsdio)

set_target_properties(rsdioctl
    PROPERTIES INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/"
)