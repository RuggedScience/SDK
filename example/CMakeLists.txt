include_directories("${PROJECT_BINARY_DIR}")
add_executable(exampledio exampledio.cpp)
add_executable(examplepoe examplepoe.cpp)

set_target_properties(exampledio PROPERTIES COMPILE_FLAGS -DRSDIO_STATIC_DEFINE)
set_target_properties(examplepoe PROPERTIES COMPILE_FLAGS -DRSPOE_STATIC_DEFINE)

target_link_libraries (exampledio rsdio-static)
target_link_libraries (examplepoe rspoe-static)