
include_directories( ../../include )

add_executable( headerLibraryTest
   runtest.cpp
   ../helputil.cpp
   basic_command_t.cpp
   )

target_link_libraries( headerLibraryTest 
   gtest_main
   )

add_test(
  NAME
    headerlib
  COMMAND
    ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/test/headerlib/headerLibraryTest
)

