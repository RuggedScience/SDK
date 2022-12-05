include(GNUInstallDirs)

if (CMAKE_VERSION VERSION_LESS "3.0")
    project(rserrors)
else()
    project(rserrors DESCRIPTION "Rugged Science Error Library")
endif()

set(PROJECT_VERSION 1.0.0)

cmake_policy(SET CMP0063 NEW)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set(CMAKE_DEBUG_POSTFIX "d")

add_library(rserrors ${CMAKE_CURRENT_SOURCE_DIR}/src/rserrors.cpp)

set_target_properties(rserrors PROPERTIES 
    VERSION ${PROJECT_VERSION}
    #POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS}
)

include(GenerateExportHeader)
generate_export_header(rserrors EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/exports/rserrors_export.h)

target_include_directories(rserrors PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/exports
)

install(TARGETS rserrors 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME} COMPONENT devel)
    
install(FILES include/rserrors.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/exports/rserrors_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME})
