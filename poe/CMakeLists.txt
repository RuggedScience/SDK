include(GNUInstallDirs)

if (CMAKE_VERSION VERSION_LESS "3.0")
    project(rspoe)
else()
    project(rspoe DESCRIPTION "Rugged Science PoE Library")
endif()

set(PROJECT_VERSION 3.0.5)

cmake_policy(SET CMP0063 NEW)

option(BUILD_SHARED_LIBS "build as shared library" ON)

set(includedir 
    ${PROJECT_SOURCE_DIR}/include
    ${rssdk_SOURCE_DIR}/utils
)
set(srcdir ${PROJECT_SOURCE_DIR}/src)
set(src 
    ${srcdir}/rspoeimpl.cpp 
    ${srcdir}/controllers/pd69104.cpp 
    ${srcdir}/controllers/pd69200.cpp 
    ${srcdir}/controllers/ltc4266.cpp 
    ${rssdk_SOURCE_DIR}/utils/tinyxml2.cpp 
    ${rssdk_SOURCE_DIR}/utils/i801_smbus.cpp
)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set(CMAKE_DEBUG_POSTFIX "d")

add_library(rspoe ${src})

set_target_properties(rspoe PROPERTIES 
    COMPILE_DEFINITIONS "RSPOE_VERSION_STRING=\"${PROJECT_VERSION}\""
    VERSION ${PROJECT_VERSION}
    POSITION_INDEPENDENT_CODE ON
)

include(GenerateExportHeader)
generate_export_header(rspoe EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/exports/rspoe_export.h)

target_link_libraries(rspoe PRIVATE rserrors)

target_include_directories(rspoe PRIVATE
    ${rssdk_SOURCE_DIR}/utils
)

target_include_directories(rspoe PUBLIC
    ${includedir}
    ${CMAKE_CURRENT_BINARY_DIR}/exports
    ${rserrors_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_definitions(rspoe PUBLIC -D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

install(TARGETS rspoe 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME} COMPONENT devel)

install(FILES include/rspoe.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/exports/rspoe_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME})
