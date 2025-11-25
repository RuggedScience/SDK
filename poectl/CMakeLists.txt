add_executable(rspoectl 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rspoectl.cpp
)

target_link_libraries(rspoectl rspoe)

set_target_properties(rspoectl
    PROPERTIES INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/"
)