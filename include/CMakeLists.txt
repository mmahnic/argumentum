
if( ARGUMENTUM_IS_TOP_LEVEL )
   file( GLOB copied_headers "${CMAKE_SOURCE_DIR}/src/*.h" )

   add_custom_command(
      OUTPUT
         ${CMAKE_CURRENT_BINARY_DIR}/fake_create_headers.cpp
         ${CMAKE_CURRENT_BINARY_DIR}/argumentum/argparse.h
      DEPENDS
         ${CMAKE_CURRENT_SOURCE_DIR}/argumentum/argparse.h
         ${copied_headers}

      COMMENT "Preparing library headers for publishing"

      COMMAND ${CMAKE_COMMAND}
          -E copy
          "${Argumentum_SOURCE_DIR}/util/faketarget.cpp"
          "${CMAKE_CURRENT_BINARY_DIR}/fake_create_headers.cpp"

      COMMAND ${CMAKE_COMMAND}
          -D TOP_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
          -D P_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
          -D P_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
          -D P_BUILDSTATIC="${ARGUMENTUM_BUILD_STATIC_LIBS}"
          -D P_HEADERONLY="${ARGUMENTUM_INSTALL_HEADERONLY}"
          -P "${CMAKE_CURRENT_SOURCE_DIR}/prepare-headers.cmake"
      )

   add_executable( fake_create_headers
      ${CMAKE_CURRENT_BINARY_DIR}/fake_create_headers.cpp
      )

   install( DIRECTORY ${CMAKE_BINARY_DIR}/include/argumentum
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      FILES_MATCHING PATTERN "*.h"
      PATTERN "inc/*_impl.h" EXCLUDE
      PATTERN "inc/*.txt" EXCLUDE
      PATTERN "inc/*.cpp" EXCLUDE
      )

   if ( ${ARGUMENTUM_INSTALL_HEADERONLY} )
      install( DIRECTORY ${CMAKE_BINARY_DIR}/include/argumentum
         DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
         FILES_MATCHING PATTERN "*_impl.h"
         )
   endif()
endif()

# When Argumentum is a Git submodule or when the header-only version is
# installed it can be used as header-only through a CMake target.
if( NOT ARGUMENTUM_IS_TOP_LEVEL OR ARGUMENTUM_INSTALL_HEADERONLY  )
   set( headeronly_name headers )
   add_library( ${headeronly_name} INTERFACE )
   add_library( ${PROJECT_NAME}::headers ALIAS ${headeronly_name} )

   if( NOT ARGUMENTUM_IS_TOP_LEVEL )
      target_include_directories( ${headeronly_name}
         INTERFACE
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
         )
   endif()

   if( ARGUMENTUM_INSTALL_HEADERONLY )
      set( target_list_name ${PROJECT_NAME}Targets )
      target_include_directories( ${headeronly_name}
         INTERFACE
         $<INSTALL_INTERFACE:include>  # <prefix>/include
         )
      install( TARGETS ${headeronly_name}
         EXPORT ${target_list_name}
         )
      set( _argumentum_has_exported_targets TRUE PARENT_SCOPE )
   endif()
endif()

