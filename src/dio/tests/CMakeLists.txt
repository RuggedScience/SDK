add_executable(rsdioimpl_test 
    test_rsdio.cpp
)

target_link_libraries(rsdioimpl_test PRIVATE 
    rsdio_impl  
    GTest::gtest 
    GTest::gmock 
    GTest::gtest_main
)

target_compile_definitions(rsdioimpl_test PRIVATE NO_EXPORT)

include(GoogleTest)
gtest_discover_tests(rsdioimpl_test)