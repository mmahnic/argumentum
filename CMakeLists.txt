cmake_minimum_required(VERSION 3.1)

project(CppArgparse VERSION 0.1.0)

option( CPPARGPARSE_BUILD_EXAMPLES   "build examples" ON )
option( CPPARGPARSE_BUILD_TESTS      "build tests"    ON )
option( CPPARGPARSE_INSTALL          "enable library installation"    ON )
option( CPPARGPARSE_PEDANTIC         "treat warnings as errors"    OFF )

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

if (CPPARGPARSE_INSTALL)
   include(CMakePackageConfigHelpers) 

   set(INCLUDE_INSTALL_DIR include/ )
   set(LIB_INSTALL_DIR lib/ )

   install( TARGETS ${CPPARGPARSE_STATIC_NAME}
      ARCHIVE DESTINATION lib
      LIBRARY DESTINATION lib
      )
   install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

   configure_package_config_file(cmake/CppArgparseConfig.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/CppArgparseConfig.cmake
      INSTALL_DESTINATION ${LIB_INSTALL_DIR}/CppArgparse/cmake
      PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR )

   write_basic_package_version_file(
      ${CMAKE_CURRENT_BINARY_DIR}/CppArgparseConfigVersion.cmake
      VERSION ${PROJECT_VERSION}
      COMPATIBILITY AnyNewerVersion
      )
endif()
