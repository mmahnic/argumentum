
set( static_library_name argumentum )

# The published static library
if ( ARGUMENTUM_BUILD_STATIC_LIBS )
   add_library( ${static_library_name} STATIC "" )
   add_library( Argumentum::${static_library_name} ALIAS ${static_library_name} )

   target_sources( ${static_library_name}
      PRIVATE
      argparser.cpp
      )

   target_include_directories( ${static_library_name}
      PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
      $<INSTALL_INTERFACE:include>  # <prefix>/include
      )

   if( ARGUMENTUM_PEDANTIC )
      target_compile_options( ${static_library_name}
         PRIVATE
         $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Werror -Wl,--fatal-warnings>
         $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /permissive- /Za>
         )
   endif()

   install( TARGETS ${static_library_name}
      EXPORT ArgumentumTargets
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      )
   set( _argumentum_has_exported_targets TRUE PARENT_SCOPE )
endif()

if( ARGUMENTUM_IS_TOP_LEVEL )
   set( internal_library_name ${_ARGUMENTUM_INTERNAL_NAME} )

   # The static library used for internal purposes: instutil, tests, examples.
   add_library( ${internal_library_name} STATIC "" )
   target_sources( ${internal_library_name}
      PRIVATE
      argparser.cpp
      )

   if( ARGUMENTUM_PEDANTIC )
      target_compile_options( ${internal_library_name}
         PRIVATE
         $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic -Werror -Wl,--fatal-warnings>
         $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /permissive- /Za>
         )
   endif()
endif()
