add_executable(exampledio exampledio.cpp)
add_executable(examplepoe examplepoe.cpp)

target_include_directories(exampledio PRIVATE "${PROJECT_BINARY_DIR}/dio")
target_include_directories(examplepoe PRIVATE "${PROJECT_BINARY_DIR}/poe")

target_link_libraries (exampledio rsdio)
target_link_libraries (examplepoe rspoe)