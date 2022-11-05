include(GNUInstallDirs)

if (CMAKE_VERSION VERSION_LESS "3.0")
    project(rsdio)
else()
    project(rsdio DESCRIPTION "Rugged Science DIO Library")
endif()

set(PROJECT_VERSION 3.0.4)

option(BUILD_SHARED_LIBS "build as shared library" ON)

set(includedir ${PROJECT_SOURCE_DIR}/include)
set(srcdir ${PROJECT_SOURCE_DIR}/src)
set(src 
    ${srcdir}/rsdioimpl.cpp 
    ${srcdir}/controllers/ite8783.cpp 
    ${srcdir}/controllers/ite8786.cpp 
    ${rssdk_SOURCE_DIR}/utils/tinyxml2.cpp
)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN1)

set(CMAKE_DEBUG_POSTFIX "d")

add_library(rsdio ${src})

set_target_properties(rsdio PROPERTIES 
    COMPILE_DEFINITIONS "RSDIO_VERSION_STRING=\"${PROJECT_VERSION}\""
    VERSION ${PROJECT_VERSION}
    POSITION_INDEPENDENT_CODE ON
)

include(GenerateExportHeader)
generate_export_header(rsdio EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/exports/rsdio_export.h)

target_include_directories(rsdio PRIVATE
    ${rssdk_SOURCE_DIR}/utils
)

target_include_directories(rsdio PUBLIC
    ${includedir}
    ${CMAKE_CURRENT_BINARY_DIR}/exports
)

if(MSVC)
    target_compile_definitions(rsdio PUBLIC -D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

install(TARGETS rsdio 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME} COMPONENT devel)
    
install(FILES include/rsdio.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/exports/rsdio_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME})
