include(GenerateExportHeader)
set(diosrc dio.cpp controllers/ite8783.cpp controllers/ite8786.cpp ../utils/tinyxml2.cpp)

add_library(rsdio SHARED ${diosrc})
add_library(rsdio-static STATIC ${diosrc})

set_property(TARGET rsdio PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET rsdio-static PROPERTY POSITION_INDEPENDENT_CODE 1)

target_include_directories(rsdio PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(rsdio-static PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

generate_export_header(rsdio)
set_target_properties(rsdio PROPERTIES COMPILE_FLAGS -DBUILD_RSDIO_SHARED)
set_target_properties(rsdio-static PROPERTIES COMPILE_FLAGS -DRSDIO_STATIC_DEFINE)