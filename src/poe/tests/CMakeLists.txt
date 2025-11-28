add_executable(rspoeimpl_test 
    test_rspoe.cpp
)

target_link_libraries(rspoeimpl_test PRIVATE 
    rspoe_impl 
    GTest::gtest 
    GTest::gmock 
    GTest::gtest_main
)

target_compile_definitions(rspoeimpl_test PRIVATE NO_EXPORT)

include(GoogleTest)
gtest_discover_tests(rspoeimpl_test)