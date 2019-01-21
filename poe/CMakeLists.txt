include(GNUInstallDirs)
include(GenerateExportHeader)
option(BUILD_SHARED_LIBS "build as shared library" ON)

set(poesrc rspoe.cpp controllers/pd69104.cpp controllers/pd69200.cpp ../utils/tinyxml2.cpp ../utils/i801_smbus.cpp)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN1)

set(CMAKE_DEBUG_POSTFIX "d")

add_library(rspoe ${poesrc})

target_include_directories(rspoe PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

generate_export_header(rspoe)

if(MSVC)
    target_compile_definitions(rspoe PUBLIC -D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_INSTALL_LIBDIR lib)
    set(CMAKE_INSTALL_INCLUDEDIR include)
endif(MSVC)

install(TARGETS rspoe DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES rspoe.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rspoe_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})