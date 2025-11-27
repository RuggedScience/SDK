pybind11_add_module(rssdkpy MODULE rssdk.cpp)

target_link_libraries(rssdkpy PRIVATE rsdio rspoe)

set_target_properties(rssdkpy PROPERTIES OUTPUT_NAME "rssdk")

find_package(Python COMPONENTS Interpreter REQUIRED)

install(TARGETS rssdkpy DESTINATION rssdk)

#************#
# Stub Files #
# ***********#
add_custom_command(TARGET rssdkpy POST_BUILD
  COMMAND ${Python_EXECUTABLE} -m pybind11_stubgen rssdk -o .
  COMMAND ${CMAKE_COMMAND} -E touch py.typed
  WORKING_DIRECTORY $<TARGET_FILE_DIR:rssdkpy>
  COMMENT "Generating Python stub files"
  VERBATIM
)

  install(FILES 
    $<TARGET_FILE_DIR:rssdkpy>/rssdk.pyi
    $<TARGET_FILE_DIR:rssdkpy>/py.typed
    DESTINATION rssdk
  )

if (WIN32)
  install(FILES ${DRIVER_FILE} DESTINATION rssdk)
endif()

