add_executable(rserrors_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_rserrors.cpp
)
target_link_libraries(rserrors_test PRIVATE rstest_utils rserrors rsdio rspoe)
add_test(NAME rserrors_test COMMAND rserrors_test)
