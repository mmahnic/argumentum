
set( static_library_name ${ARGUMENTUM_STATIC_NAME} )
set( internal_library_name ${ARGUMENTUM_INTERNAL_NAME} )
set( target_list_name ${PROJECT_NAME}Targets )

# When Argumentum is a Git submodule it can be used as header-only through a
# CMake target.
if( NOT ARGUMENTUM_IS_TOP_LEVEL )
   set( headeronly_name __argumentum_hpp__ )
   add_library( ${headeronly_name} INTERFACE )
   add_library( Argumentum::header ALIAS ${headeronly_name} )

   target_include_directories( ${headeronly_name}
      INTERFACE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
      )
endif()

# The published static library
if ( ARGUMENTUM_BUILD_STATIC_LIBS )
   add_library( ${static_library_name} STATIC "" )
   add_library( Argumentum::static ALIAS ${static_library_name} )

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
   add_library( ${internal_library_name} STATIC "" )
   target_sources( ${internal_library_name}
      PRIVATE
      argparser.cpp
      )

   if( ARGUMENTUM_PEDANTIC )
      target_compile_options( ${internal_library_name}
         PRIVATE
         $<$<CXX_COMPILER_ID:GNU>:-Werror -Wall>
         $<$<CXX_COMPILER_ID:MSVC>:/WX /permissive- /Za>
         )
   endif()
endif()
