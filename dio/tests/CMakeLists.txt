# implementation test
add_executable(rsdioimpl_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_rsdioimpl.cpp
)
target_compile_definitions(rsdioimpl_test PRIVATE NO_EXPORT)
target_link_libraries(rsdioimpl_test PRIVATE rstest_utils rsdio_impl)
add_test(NAME rsdioimpl_test COMMAND rsdioimpl_test)

# rsdio test
add_executable(rsdio_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_rsdio.cpp
)
target_link_libraries(rsdio_test PRIVATE rstest_utils rserrors rsdio)
add_test(NAME rsdio_test COMMAND rsdio_test)
set_tests_properties(rsdio_test PROPERTIES 
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
)