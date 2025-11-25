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

set_target_properties(rserrors PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH FALSE
)

# -----------------------------------------------------------------------------
# Install logic
# -----------------------------------------------------------------------------
if (INSTALL_SDK)
    install(TARGETS rserrors EXPORT rssdk_Targets
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
    
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/rserrors_export.h"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rssdk
        COMPONENT rssdk_Development
    )
endif()

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()