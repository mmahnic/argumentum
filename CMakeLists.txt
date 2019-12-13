cmake_minimum_required(VERSION 3.1)

project(CppArgparse VERSION 0.1.0)

option( CPPARGPARSE_BUILD_EXAMPLES   "build examples" ON )
option( CPPARGPARSE_BUILD_TESTS      "build tests"    ON )
option( CPPARGPARSE_INSTALL          "enable library installation"    ON )
option( CPPARGPARSE_PEDANTIC         "treat warnings as errors"    OFF )

set(CMAKE_CXX_STANDARD 17)
set(INCLUDE_INSTALL_DIR include/ )
set(LIB_INSTALL_DIR lib/ )
set(CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/CppArgparse)

set(CPPARGPARSE_STATIC_NAME argparse-s)
add_subdirectory(src)
add_subdirectory(util)

if( CPPARGPARSE_BUILD_EXAMPLES )
   add_subdirectory(example)
endif()

if( CPPARGPARSE_BUILD_TESTS )
   enable_testing()
   add_subdirectory(test)
   add_subdirectory(test/staticlib)
   add_subdirectory(test/headerlib)
endif()

if (CPPARGPARSE_INSTALL)
   include(GNUInstallDirs)
   add_subdirectory(include)

   include(CMakePackageConfigHelpers)

   configure_package_config_file(cmake/CppArgparseConfig.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/CppArgparseConfig.cmake
      INSTALL_DESTINATION ${LIB_INSTALL_DIR}/CppArgparse/cmake
      PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR )

   write_basic_package_version_file(
      ${CMAKE_CURRENT_BINARY_DIR}/CppArgparseConfigVersion.cmake
      VERSION ${PROJECT_VERSION}
      COMPATIBILITY AnyNewerVersion
      )

   install(FILES
      ${CMAKE_CURRENT_BINARY_DIR}/CppArgparseConfig.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/CppArgparseConfigVersion.cmake
      DESTINATION ${CONFIG_INSTALL_DIR}
      )

endif()
