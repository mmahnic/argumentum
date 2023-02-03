cmake_minimum_required( VERSION 3.13 )

project( Argumentum VERSION 0.3.2 )
if( CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME )
   set( ARGUMENTUM_IS_TOP_LEVEL TRUE )
else()
   set( ARGUMENTUM_IS_TOP_LEVEL FALSE )
endif()

set( _build_static_libs ${ARGUMENTUM_IS_TOP_LEVEL} )

option( ARGUMENTUM_BUILD_STATIC_LIBS  "Build static libraries" ${_build_static_libs} )
option( ARGUMENTUM_DEPRECATED_ATTR    "Enable deprecation attributes"      OFF )
option( ARGUMENTUM_PEDANTIC           "Treat warnings as errors (TODO)"    OFF )

if( BUILD_SHARED_LIBS )
   message( FATAL_ERROR "Shared libries are not supported ATM" )
endif()

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_DEBUG_POSTFIX d )

include( GNUInstallDirs )

# The name of the published static library target.
set( ARGUMENTUM_STATIC_NAME argumentum )

# The name of the internal static library target used for tests, examples.
set( ARGUMENTUM_INTERNAL_NAME argumentum-si )

if( ARGUMENTUM_DEPRECATED_ATTR )
   add_definitions( -DARGUMENTUM_DEPRECATED_ATTR )
endif()

add_subdirectory( src )
add_subdirectory( include )

include( cmake/InstallConfig.cmake )

if( ARGUMENTUM_IS_TOP_LEVEL )
   option( ARGUMENTUM_BUILD_EXAMPLES   "Build examples" OFF )
   option( ARGUMENTUM_BUILD_TESTS      "Build tests"    OFF )

   if( ARGUMENTUM_BUILD_EXAMPLES )
      add_subdirectory( example )
   endif()

   if( ARGUMENTUM_BUILD_TESTS )
      enable_testing()
      add_subdirectory( test )
   endif()
endif()
