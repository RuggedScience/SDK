if (CMAKE_VERSION VERSION_LESS "3.0")
    project(rserrors)
else()
    project(rserrors DESCRIPTION "Rugged Science Error Library")
endif()

set(PROJECT_VERSION 1.0.0)

add_library(rserrors STATIC ${PROJECT_SOURCE_DIR}/src/rserrors.cpp)

set_target_properties(rserrors PROPERTIES 
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(rserrors PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

install(FILES include/rserrors.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME})