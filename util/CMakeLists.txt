
include_directories( ../include )

set(CMAKE_CXX_STANDARD 17)

add_executable(instutil
   instutil.cpp
   )

if( ARGUMENTUM_PEDANTIC )
   target_compile_options(${ARGUMENTUM_STATIC_NAME} PRIVATE 
      $<$<CXX_COMPILER_ID:GNU>:-Werror -Wall>
      $<$<CXX_COMPILER_ID:MSVC>:/WX /permissive- /Za>
      )
endif()

target_link_libraries( instutil
   argparse-s
   )
