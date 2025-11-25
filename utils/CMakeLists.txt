add_library(rsutils STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tinyxml2.cpp
)
target_include_directories(rsutils PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

# Since this is internal, we generally don't install it, 
# but we do need PIC if it's linking into a shared lib.
set_target_properties(rsutils PROPERTIES POSITION_INDEPENDENT_CODE ON)