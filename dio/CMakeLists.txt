include(GNUInstallDirs)
include(GenerateExportHeader)
option(BUILD_SHARED_LIBS "build as shared library" ON)
project(rsdio VERSION 3.0.0 DESCRIPTION "Rugged Science DIO LIBRARY")

set(diosrc rsdioimpl.cpp controllers/ite8783.cpp controllers/ite8786.cpp ../utils/tinyxml2.cpp)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN1)

set(CMAKE_DEBUG_POSTFIX "d")

add_library(rsdio ${diosrc})

set_property(TARGET rsdio PROPERTY CXX_STANDARD 11)
set_target_properties(rsdio PROPERTIES COMPILE_DEFINITIONS "RSDIO_VERSION_STRING=\"${PROJECT_VERSION}\"")
set_target_properties(rsdio PROPERTIES  VERSION ${PROJECT_VERSION}
                                        SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(rsdio PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

generate_export_header(rsdio)

if(MSVC)
    target_compile_definitions(rsdio PUBLIC -D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

install(TARGETS rsdio 
    RUNTIME DESTINATION bin COMPONENT runtime
    LIBRARY DESTINATION lib COMPONENT devel
    ARCHIVE DESTINATION lib COMPONENT devel
    PUBLIC_HEADER DESTINATION include COMPONENT devel)
    
install(FILES rsdio.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rsdio_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})