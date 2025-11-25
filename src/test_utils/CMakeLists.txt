add_library(rstest_utils STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_utils.cpp
)
target_include_directories(rstest_utils PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

# Since this is internal, we generally don't install it, 
# but we do need PIC if it's linking into a shared lib.
set_target_properties(rstest_utils PROPERTIES POSITION_INDEPENDENT_CODE ON)