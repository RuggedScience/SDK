include(GNUInstallDirs)
include(GenerateExportHeader)
option(BUILD_SHARED_LIBS "build as shared library" ON)
project(rspoe VERSION 3.0.0 DESCRIPTION "Rugged Science PoE LIBRARY")

set(poesrc rspoeimpl.cpp controllers/pd69104.cpp controllers/pd69200.cpp controllers/ltc4266.cpp ../utils/tinyxml2.cpp ../utils/i801_smbus.cpp)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN1)

set(CMAKE_DEBUG_POSTFIX "d")

add_library(rspoe ${poesrc})

set_property(TARGET rspoe PROPERTY CXX_STANDARD 11)
set_target_properties(rspoe PROPERTIES COMPILE_DEFINITIONS "RSPOE_VERSION_STRING=\"${PROJECT_VERSION}\"")
set_target_properties(rspoe PROPERTIES  VERSION ${PROJECT_VERSION}
                                        SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(rspoe PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

generate_export_header(rspoe)

if(MSVC)
    target_compile_definitions(rspoe PUBLIC -D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

install(TARGETS rspoe 
    RUNTIME DESTINATION bin COMPONENT runtime
    LIBRARY DESTINATION lib COMPONENT devel
    ARCHIVE DESTINATION lib COMPONENT devel
    PUBLIC_HEADER DESTINATION include COMPONENT devel)

install(FILES rspoe.h DESTINATION include)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rspoe_export.h DESTINATION include)