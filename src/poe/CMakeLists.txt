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

target_link_libraries(rspoe_impl PUBLIC core config PRIVATE portio)
set_target_properties(rspoe_impl PROPERTIES POSITION_INDEPENDENT_CODE ON)

# -----------------------------------------------------------------------------
# PUBLIC LIBRARY
# Contains the Entry Point (Factory) and exports the API.
# -----------------------------------------------------------------------------
add_library(rspoe
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rspoe.cpp
)
target_link_libraries(rspoe PRIVATE rspoe_impl INTERFACE core)

target_include_directories(rspoe 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)
target_compile_definitions(
    rspoe PUBLIC $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:RSPOE_STATIC_DEFINE>
)

include(GenerateExportHeader)
generate_export_header(rspoe EXPORT_FILE_NAME include/rspoe_export.h)

set_target_properties(rspoe 
    PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        BUILD_WITH_INSTALL_RPATH FALSE
)

# -----------------------------------------------------------------------------
# Install logic
# -----------------------------------------------------------------------------
if (INSTALL_SDK)
    install(TARGETS rspoe EXPORT rssdk_Targets
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
    
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/rspoe_export.h"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rssdk
        COMPONENT rssdk_Development
    )
endif()


# Only add the test subdirectory if testing is enabled
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()