
include_directories( ../../include )

find_package( GTest REQUIRED )
find_package( Threads REQUIRED )

add_executable( staticLibraryTest
   runtest.cpp
   ../testutil.cpp
   basic_command_t.cpp
   )

target_link_libraries( staticLibraryTest 
   ${GTEST_LIBRARIES}
   ${CMAKE_THREAD_LIBS_INIT}
   ${ARGUMENTUM_STATIC_NAME}
   )
add_dependencies( staticLibraryTest ${ARGUMENTUM_STATIC_NAME} )

add_test(
  NAME
    staticlib
  COMMAND
    ${CMAKE_BINARY_DIR}/test/staticlib/staticLibraryTest
)

