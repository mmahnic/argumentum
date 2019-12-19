cmake_minimum_required(VERSION 3.1)

project(Argumentum VERSION 0.2.0)

option( ARGUMENTUM_BUILD_EXAMPLES   "build examples" ON )
option( ARGUMENTUM_BUILD_TESTS      "build tests"    ON )
option( ARGUMENTUM_INSTALL          "enable library installation"    ON )
option( ARGUMENTUM_PEDANTIC         "treat warnings as errors"    OFF )

set(CMAKE_CXX_STANDARD 17)
set(INCLUDE_INSTALL_DIR include/ )
set(LIB_INSTALL_DIR lib/ )
set(CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/Argumentum)

set(ARGUMENTUM_STATIC_NAME argumentum-s)
add_subdirectory(src)
add_subdirectory(util)

if( ARGUMENTUM_BUILD_EXAMPLES )
   add_subdirectory(example)
endif()

if( ARGUMENTUM_BUILD_TESTS )
   enable_testing()
   add_subdirectory(test)
   add_subdirectory(test/staticlib)
   add_subdirectory(test/headerlib)
endif()

if (ARGUMENTUM_INSTALL)
   include(GNUInstallDirs)
   add_subdirectory(include)

   include(CMakePackageConfigHelpers)

   configure_package_config_file(cmake/ArgumentumConfig.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/ArgumentumConfig.cmake
      INSTALL_DESTINATION ${LIB_INSTALL_DIR}/Argumentum/cmake
      PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR )

   write_basic_package_version_file(
      ${CMAKE_CURRENT_BINARY_DIR}/ArgumentumConfigVersion.cmake
      VERSION ${PROJECT_VERSION}
      COMPATIBILITY AnyNewerVersion
      )

   install(FILES
      ${CMAKE_CURRENT_BINARY_DIR}/ArgumentumConfig.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/ArgumentumConfigVersion.cmake
      DESTINATION ${CONFIG_INSTALL_DIR}
      )

endif()
