# -----------------------------------------------------------------------------
# INTERNAL LIBRARY
# Contains the implementation logic. Hidden from the public. Used for testing
# -----------------------------------------------------------------------------
add_library(rspoe_impl STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rspoeimpl.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controllers/pd69104.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controllers/pd69200.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controllers/ltc4266.cpp 
)
target_compile_definitions(rspoe_impl PRIVATE NO_EXPORT)
target_include_directories(rspoe_impl PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
)

target_link_libraries(rspoe_impl PUBLIC rserrors PRIVATE rsportio rsutils)
set_target_properties(rspoe_impl PROPERTIES POSITION_INDEPENDENT_CODE ON)

# -----------------------------------------------------------------------------
# PUBLIC LIBRARY
# Contains the Entry Point (Factory) and exports the API.
# -----------------------------------------------------------------------------
add_library(rspoe
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rspoe.cpp
)
target_link_libraries(rspoe PRIVATE rspoe_impl)

target_include_directories(
    rspoe PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
target_compile_definitions(
    rspoe PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:RSPOE_STATIC_DEFINE>"
)

include(GenerateExportHeader)
generate_export_header(rspoe EXPORT_FILE_NAME include/rspoe_export.h)

target_include_directories(
    rspoe PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
)

# Only add the test subdirectory if testing is enabled
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()