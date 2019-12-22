
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

