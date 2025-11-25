add_library(rsportio STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/portio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i801_smbus.cpp
)
target_include_directories(rsportio PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

# Since this is internal, we generally don't install it, 
# but we do need PIC if it's linking into a shared lib.
set_target_properties(rsportio PROPERTIES POSITION_INDEPENDENT_CODE ON)