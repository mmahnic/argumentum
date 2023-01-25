
set( static_library_name ${ARGUMENTUM_STATIC_NAME} )
set( internal_library_name ${ARGUMENTUM_INTERNAL_NAME} )
set( target_list_name ${PROJECT_NAME}Targets )

# The published static library
if ( BUILD_STATIC_LIBS )
   add_library( ${static_library_name} STATIC
      argparser.cpp
      )
   if( ARGUMENTUM_PEDANTIC )
      target_compile_options( ${static_library_name} PRIVATE
         $<$<CXX_COMPILER_ID:GNU>:-Werror -Wall>
         $<$<CXX_COMPILER_ID:MSVC>:/WX /permissive- /Za>
         )
   endif()

   install( TARGETS ${static_library_name}
      EXPORT ${target_list_name}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      )
endif()

if( ARGUMENTUM_IS_TOP_LEVEL )
   # The static library used for internal purposes: instutil, tests, examples.
   add_library( ${internal_library_name} STATIC
      argparser.cpp
      )
endif()

if( ARGUMENTUM_PEDANTIC )
   target_compile_options( ${internal_library_name} PRIVATE
      $<$<CXX_COMPILER_ID:GNU>:-Werror -Wall>
      $<$<CXX_COMPILER_ID:MSVC>:/WX /permissive- /Za>
      )
endif()
