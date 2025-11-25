# -----------------------------------------------------------------------------
# INTERNAL LIBRARY
# Contains the implementation logic. Hidden from the public. Used for testing
# -----------------------------------------------------------------------------
add_library(rsdio_impl STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rsdioimpl.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controllers/ite8783.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controllers/ite8786.cpp 
)
target_compile_definitions(rsdio_impl PRIVATE NO_EXPORT)

target_include_directories(rsdio_impl PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
)

target_link_libraries(rsdio_impl PUBLIC rserrors PRIVATE rsportio rsutils)
set_target_properties(rsdio_impl PROPERTIES POSITION_INDEPENDENT_CODE ON)

# -----------------------------------------------------------------------------
# PUBLIC LIBRARY
# Contains the Entry Point (Factory) and exports the API.
# -----------------------------------------------------------------------------
add_library(rsdio
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rsdio.cpp
)

target_link_libraries(rsdio PRIVATE rsdio_impl)

target_include_directories(
    rsdio PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

target_compile_definitions(
    rsdio PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:RSDIO_STATIC_DEFINE>"
)

include(GenerateExportHeader)
generate_export_header(rsdio EXPORT_FILE_NAME include/rsdio_export.h)

target_include_directories(
    rsdio PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
)

set_target_properties(rsdio PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH FALSE
)

# -----------------------------------------------------------------------------
# Install logic
# -----------------------------------------------------------------------------
if (INSTALL_SDK)
    install(TARGETS rsdio EXPORT rssdk_Targets
        RUNTIME COMPONENT rssdk_Runtime
        LIBRARY COMPONENT rssdk_Runtime
        NAMELINK_COMPONENT rssdk_Development
        ARCHIVE COMPONENT rssdk_Development
        INCLUDES COMPONENT rssdk_Development
    )

    install(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rssdk
        COMPONENT rssdk_Development
        FILES_MATCHING PATTERN "*.h" # or *.hpp
    )
    
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/rsdio_export.h"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rssdk
        COMPONENT rssdk_Development
    )
endif()

# Only add the test subdirectory if testing is enabled
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()

