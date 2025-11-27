add_library(config 
    STATIC 
        src/tinyxml2.cpp
        src/abstractconfigparser.cpp
        src/xmlparser.cpp
        src/rsparser.cpp
        src/vecowparser.cpp
        src/configparser.cpp
        
)

target_include_directories(config
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(config PUBLIC core)

# Since this is internal, we generally don't install it, 
# but we do need PIC if it's linking into a shared lib.
set_target_properties(config PROPERTIES POSITION_INDEPENDENT_CODE ON)