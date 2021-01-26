if (CMAKE_VERSION VERSION_LESS "3.0")
    project(rspoe)
else()
    project(rspoe DESCRIPTION "Rugged Science PoE Library")
endif()

set(PROJECT_VERSION 3.0.4)

include(GNUInstallDirs)
include(GenerateExportHeader)
option(BUILD_SHARED_LIBS "build as shared library" ON)

set(poesrc rspoeimpl.cpp controllers/pd69104.cpp controllers/pd69200.cpp controllers/ltc4266.cpp ../utils/tinyxml2.cpp ../utils/i801_smbus.cpp)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN1)

set(CMAKE_DEBUG_POSTFIX "d")

add_library(rspoe ${poesrc})

set_target_properties(rspoe PROPERTIES COMPILE_DEFINITIONS "RSPOE_VERSION_STRING=\"${PROJECT_VERSION}\"")
set_target_properties(rspoe PROPERTIES  VERSION ${PROJECT_VERSION})

target_include_directories(rspoe PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

generate_export_header(rspoe)

if(MSVC)
    target_compile_definitions(rspoe PUBLIC -D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

install(TARGETS rspoe 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME} COMPONENT devel)

install(FILES rspoe.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rspoe_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME})
