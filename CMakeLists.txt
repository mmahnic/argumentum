cmake_minimum_required( VERSION 3.1 )

project( Argumentum VERSION 0.2.0 )

option( ARGUMENTUM_BUILD_EXAMPLES   "build examples" OFF )
option( ARGUMENTUM_BUILD_TESTS      "build tests"    OFF )
option( ARGUMENTUM_PEDANTIC         "treat warnings as errors (TODO)"    OFF )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_DEBUG_POSTFIX d )

include( GNUInstallDirs )

set( ARGUMENTUM_STATIC_NAME argumentum-s )
add_subdirectory( src )
add_subdirectory( util )
add_subdirectory( include )

include( cmake/InstallConfig.cmake )

if( ARGUMENTUM_BUILD_EXAMPLES )
   add_subdirectory( example )
endif()

if( ARGUMENTUM_BUILD_TESTS )
   enable_testing()
   add_subdirectory( test )
   add_subdirectory( test/staticlib )
   add_subdirectory( test/headerlib )
endif()

