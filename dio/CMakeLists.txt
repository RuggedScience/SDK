if (CMAKE_VERSION VERSION_LESS "3.0")
    project(rsdio)
else()
    project(rsdio DESCRIPTION "Rugged Science DIO Library")
endif()

set(PROJECT_VERSION 3.0.4)

include(GNUInstallDirs)
include(GenerateExportHeader)
option(BUILD_SHARED_LIBS "build as shared library" ON)

set(diosrc rsdioimpl.cpp controllers/ite8783.cpp controllers/ite8786.cpp ../utils/tinyxml2.cpp)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN1)

set(CMAKE_DEBUG_POSTFIX "d")

add_library(rsdio ${diosrc})

set_target_properties(rsdio PROPERTIES COMPILE_DEFINITIONS "RSDIO_VERSION_STRING=\"${PROJECT_VERSION}\"")
set_target_properties(rsdio PROPERTIES VERSION ${PROJECT_VERSION})

target_include_directories(rsdio PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

generate_export_header(rsdio)

if(MSVC)
    target_compile_definitions(rsdio PUBLIC -D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

install(TARGETS rsdio 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME} COMPONENT devel)
    
install(FILES rsdio.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rsdio_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME})
