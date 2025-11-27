#implementation test
add_executable(rspoeimpl_test
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_rspoeimpl.cpp
)
target_compile_definitions(rspoeimpl_test PUBLIC NO_EXPORT)
target_link_libraries(rspoeimpl_test PRIVATE rspoe_impl)
add_test(NAME rspoeimpl_test COMMAND rspoeimpl_test)

# rspoe test
add_executable(rspoe_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_rspoe.cpp
)
target_link_libraries(rspoe_test PRIVATE rspoe)
add_test(NAME rspoe_test COMMAND rspoe_test)
set_tests_properties(rspoe_test PROPERTIES 
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
)