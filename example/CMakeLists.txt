add_executable (exampledio exampledio.cpp)
target_link_libraries (exampledio PUBLIC rsdio_shared)
add_executable (examplepoe examplepoe.cpp)
target_link_libraries (examplepoe PUBLIC rspoe)
