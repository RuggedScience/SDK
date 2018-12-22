include(GenerateExportHeader)
option(BUILD_SHARED_LIBS "build as shared library" ON)

set(diosrc rsdio.cpp controllers/ite8783.cpp controllers/ite8786.cpp ../utils/tinyxml2.cpp)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN1)

set(CMAKE_DEBUG_POSTFIX "d")

add_library(rsdio ${diosrc})

target_include_directories(rsdio PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

generate_export_header(rsdio)

if(MSVC)
    target_compile_definitions(rsdio PUBLIC -D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_INSTALL_LIBDIR lib)
    set(CMAKE_INSTALL_INCLUDEDIR include)
endif(MSVC)

install(TARGETS rsdio DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES rsdio.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rsdio_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})