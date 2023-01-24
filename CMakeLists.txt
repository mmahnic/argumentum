cmake_minimum_required( VERSION 3.1 )

project( Argumentum VERSION 0.3.2 )

option( BUILD_STATIC_LIBS           "Build static libraries" ON )
option( ARGUMENTUM_DEPRECATED_ATTR  "Enable deprecation attributes"      OFF )
option( ARGUMENTUM_PEDANTIC         "Treat warnings as errors (TODO)"    OFF )

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

if ( ARGUMENTUM_DEPRECATED_ATTR )
   add_definitions( -DARGUMENTUM_DEPRECATED_ATTR )
endif()

add_subdirectory( src )
add_subdirectory( include )

include( cmake/InstallConfig.cmake )

if (PROJECT_IS_TOP_LEVEL)
   option( ARGUMENTUM_BUILD_EXAMPLES   "Build examples" OFF )
   option( ARGUMENTUM_BUILD_TESTS      "Build tests"    OFF )

   if( ARGUMENTUM_BUILD_EXAMPLES )
      add_subdirectory( example )
   endif()

   if( ARGUMENTUM_BUILD_TESTS )
      enable_testing()
      add_subdirectory( test )
      add_subdirectory( test/staticlib )
      add_subdirectory( test/headerlib )
   endif()
endif()
