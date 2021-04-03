
include_directories( ../../include )

find_package( GTest REQUIRED )
find_package( Threads REQUIRED )

include_directories(${GTEST_INCLUDE_DIRS})

add_executable( headerLibraryTest
   runtest.cpp
   ../testutil.cpp
   basic_command_t.cpp
   )

target_link_libraries( headerLibraryTest 
   ${GTEST_LIBRARIES}
   ${CMAKE_THREAD_LIBS_INIT}
   )

add_test(
  NAME
    headerlib
  COMMAND
    ${CMAKE_BINARY_DIR}/test/headerlib/headerLibraryTest
)

