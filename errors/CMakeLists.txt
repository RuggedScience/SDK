add_library(rserrors 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rserrors.cpp
)
target_include_directories(
    rserrors PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
target_compile_definitions(
    rserrors PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:RSERRORS_STATIC_DEFINE>"
)

include(GenerateExportHeader)
generate_export_header(rserrors EXPORT_FILE_NAME include/rserrors_export.h)

target_include_directories(
    rserrors PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()