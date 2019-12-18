
include_directories( ../../include )
set ( CPPARGPARSE_TEST_LIB ${CPPARGPARSE_STATIC_NAME} )

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
   ${CMAKE_BINARY_DIR}/src/libargparse-s.a
   )
add_dependencies( staticLibraryTest ${CPPARGPARSE_TEST_LIB} )

add_test(
  NAME
    staticlib
  COMMAND
    ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/test/staticlib/staticLibraryTest
)

