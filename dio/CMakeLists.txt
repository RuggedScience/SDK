include(GNUInstallDirs)
include(GenerateExportHeader)
option(BUILD_SHARED_LIBS "build as shared library" ON)

set(diosrc rsdioimpl.cpp controllers/ite8783.cpp controllers/ite8786.cpp ../utils/tinyxml2.cpp)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN1)

set(CMAKE_DEBUG_POSTFIX "d")

add_library(rsdio ${diosrc})

set(RSDIO_VERSION_MAJOR 2)
set(RSDIO_VERSION_MINOR 1)
set(RSDIO_VERSION_PATCH 0)
set(RSDIO_VERSION_STRING ${RSDIO_VERSION_MAJOR}.${RSDIO_VERSION_MINOR}.${RSDIO_VERSION_PATCH})
set_target_properties(rsdio PROPERTIES COMPILE_DEFINITIONS "RSDIO_VERSION_STRING=\"${RSDIO_VERSION_STRING}\"")

set_target_properties(rsdio PROPERTIES VERSION ${RSDIO_VERSION_STRING}
                                        SOVERSION ${RSDIO_VERSION_MAJOR})

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