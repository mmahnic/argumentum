
include_directories( ../../include )
link_directories( 
   ${CMAKE_BINARY_DIR}/src 
   )
message( "***" ${CMAKE_BINARY_DIR} )

add_executable( staticLibraryTest
   runtest.cpp
   ../helputil.cpp
   basic_command_t.cpp
   )

target_link_libraries( staticLibraryTest 
   gtest_main
   ${CMAKE_BINARY_DIR}/src/libargparse-s.a
   )

add_test(
  NAME
    staticlib
  COMMAND
    ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/test/staticlib/staticLibraryTest
)

