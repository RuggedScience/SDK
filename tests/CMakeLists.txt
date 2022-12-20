get_target_property(rserrors_SOURCES rserrors SOURCES)

get_target_property(rsdio_SOURCES rsdio SOURCES)
add_executable(rsdioimpl_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_rsdioimpl.cpp
    ${rserrors_SOURCES}
    ${rsdio_SOURCES}
)
target_compile_definitions(rsdioimpl_test PUBLIC NO_EXPORT)

get_target_property(rspoe_SOURCES rspoe SOURCES)
add_executable(rspoeimpl_test
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_rspoeimpl.cpp
    ${rserrors_SOURCES}
    ${rspoe_SOURCES}
)
target_compile_definitions(rspoeimpl_test PUBLIC NO_EXPORT)

add_executable(rsdio_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_rsdio.cpp
)
target_link_libraries(rsdio_test PRIVATE rsdio)

add_executable(rspoe_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_rspoe.cpp
)
target_link_libraries(rspoe_test PRIVATE rspoe)

add_executable(rserrors_test
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_rserrors.cpp
)
target_link_libraries(rserrors_test PRIVATE rsdio rspoe)

enable_testing()

add_test(
    NAME rserrors_test
    COMMAND rserrors_test
)

add_test(
    NAME rsdioimpl_test
    COMMAND rsdioimpl_test
)

add_test(
    NAME rspoeimpl_test
    COMMAND rspoeimpl_test
)

add_test(
    NAME rsdio_test 
    COMMAND rsdio_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

add_test(
    NAME rspoe_test 
    COMMAND rspoe_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
)
