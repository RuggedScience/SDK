include(GNUInstallDirs)
option(INSTALL_EXAMPLES "Installs examples" ON)

add_executable(rsdioctl rsdioctl.cpp)
add_executable(rspoectl rspoectl.cpp)

target_include_directories(rsdioctl PRIVATE "${PROJECT_BINARY_DIR}/dio")
target_include_directories(rspoectl PRIVATE "${PROJECT_BINARY_DIR}/poe")

target_link_libraries (rsdioctl rsdio)
target_link_libraries (rspoectl rspoe)

SET_TARGET_PROPERTIES(rsdioctl rspoectl
    PROPERTIES INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")


if(INSTALL_EXAMPLES)
    if(MSVC)
        set(CMAKE_INSTALL_BINDIR bin)
    endif(MSVC)

    install(TARGETS rsdioctl DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(TARGETS rspoectl DESTINATION ${CMAKE_INSTALL_BINDIR})
endif(INSTALL_EXAMPLES)
