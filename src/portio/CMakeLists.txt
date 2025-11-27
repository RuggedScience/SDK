add_library(portio STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/portio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i801_smbus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/smbus.cpp
)
target_include_directories(portio PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

# Since this is internal, we generally don't install it, 
# but we do need PIC if it's linking into a shared lib.
set_target_properties(portio PROPERTIES POSITION_INDEPENDENT_CODE ON)