include(GenerateExportHeader)
set(poesrc poe.cpp controllers/pd69104.cpp ../utils/tinyxml2.cpp)

add_library(rspoe SHARED ${poesrc})
add_library(rspoe-static STATIC ${poesrc})

set_property(TARGET rspoe PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET rspoe-static PROPERTY POSITION_INDEPENDENT_CODE 1)

target_include_directories(rspoe PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(rspoe-static PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

generate_export_header(rspoe)
set_target_properties(rspoe PROPERTIES COMPILE_FLAGS -DBUILD_RSPOE_SHARED)
set_target_properties(rspoe-static PROPERTIES COMPILE_FLAGS -DRSPOE_STATIC_DEFINE)

if(MSVC)
    target_compile_definitions(rspoe PUBLIC -D_CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(rspoe-static PUBLIC -D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)
