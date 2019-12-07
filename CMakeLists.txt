cmake_minimum_required(VERSION 3.1)

project(ArgParser)

option( CPPARGPARSE_BUILD_EXAMPLES "build examples" ON )
option( CPPARGPARSE_BUILD_TESTS    "build tests"    ON )

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)

if( CPPARGPARSE_BUILD_EXAMPLES )
   add_subdirectory(example)
endif()


if( CPPARGPARSE_BUILD_TESTS )
   include(cmake/googletest/googletest.cmake)
   fetch_googletest(
      ${PROJECT_SOURCE_DIR}/cmake/googletest
      ${PROJECT_BINARY_DIR}/googletest
      )

   enable_testing()

   add_subdirectory(test)
   add_subdirectory(test/staticlib)
   add_subdirectory(test/headerlib)
endif()
