
set( static_library_name ${ARGUMENTUM_STATIC_NAME} )
set( module_name ${ARGUMENTUM_MODULE_NAME} )
set( internal_library_name ${ARGUMENTUM_INTERNAL_NAME} )
set( target_list_name ${PROJECT_NAME}Targets )

# The published static library
if ( BUILD_STATIC_LIBS )
   add_library( ${static_library_name} STATIC
      argparser.cpp
      )
   target_compile_definitions(${static_library_name} PRIVATE 
        # ARGUMENTUM_EXPORT is defined when the module is being build. In all other cases
        # we have to define it separately. If we just define it here, the compiler will
        # see it as ARGUMENTUM_EXPORT=1. So we use an intermediate define and define 
        # ARGUMENTUM_EXPORT in the code (nomodule.h).
        ARGUMENTUM_DEFINE_EXPORT=true
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

if ( BUILD_MODULE )
   add_library( ${module_name} STATIC
      argumentum.ixx
      )
   set_property(TARGET ${module_name} PROPERTY CXX_STANDARD 20)
   target_compile_options( ${module_name} PRIVATE
      $<$<CXX_COMPILER_ID:MSVC>:/experimental:module>
      )
endif()

# The static library used for internal purposes: instutil, tests, examples.
add_library( ${internal_library_name} STATIC
   argparser.cpp
   )
target_compile_definitions(${internal_library_name} PRIVATE
	ARGUMENTUM_DEFINE_EXPORT=true
)
if( ARGUMENTUM_PEDANTIC )
   target_compile_options( ${internal_library_name} PRIVATE
      $<$<CXX_COMPILER_ID:GNU>:-Werror -Wall>
      $<$<CXX_COMPILER_ID:MSVC>:/WX /permissive- /Za>
      )
endif()
