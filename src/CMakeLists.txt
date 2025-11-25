add_subdirectory(portio)
add_subdirectory(errors)
add_subdirectory(utils)
add_subdirectory(dio)
add_subdirectory(poe)

if (BUILD_TESTS)
    add_subdirectory(test_utils)
endif()

if (BUILD_UTILITIES)
    add_subdirectory(dioctl)
    add_subdirectory(poectl)
endif()