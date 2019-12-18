
include_directories( ../../include )

find_package( GTest REQUIRED )
find_package( Threads REQUIRED )

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
    ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/test/headerlib/headerLibraryTest
)

