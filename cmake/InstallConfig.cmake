
include(CMakePackageConfigHelpers)

set( cmake_package_name ${PROJECT_NAME} )
set( package_version ${PROJECT_VERSION} )

set( generated_dir "${CMAKE_CURRENT_BINARY_DIR}/gen" CACHE INTERNAL "" )
set( cmake_files_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${cmake_package_name}" )
set( version_file "${generated_dir}/${cmake_package_name}ConfigVersion.cmake" )
set( config_file_in "cmake/ArgumentumConfig.cmake.in" )
set( config_file "${generated_dir}/${cmake_package_name}Config.cmake" )
set( targets_export_name ${cmake_package_name}Targets CACHE INTERNAL "" )

write_basic_package_version_file( ${version_file}
   VERSION ${package_version}
   COMPATIBILITY AnyNewerVersion
   )

if( BUILD_STATIC_LIBS )
   install(
      EXPORT ${targets_export_name}
      NAMESPACE ${cmake_package_name}::
      DESTINATION ${cmake_files_install_dir}
      )
endif()

configure_package_config_file(
   ${config_file_in}
   ${config_file}
   INSTALL_DESTINATION ${cmake_files_install_dir}
   )

install(
   FILES ${version_file} ${config_file}
   DESTINATION ${cmake_files_install_dir}
   )

