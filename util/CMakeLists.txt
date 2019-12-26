
include_directories( ../include )
set ( argumentum_util_lib ${ARGUMENTUM_STATIC_NAME} )

set(CMAKE_CXX_STANDARD 17)

add_executable( instutil
   instutil.cpp
   )

if( ARGUMENTUM_PEDANTIC )
   target_compile_options( instutil PRIVATE
      $<$<CXX_COMPILER_ID:GNU>:-Werror -Wall>
      $<$<CXX_COMPILER_ID:MSVC>:/WX /permissive- /Za>
      )
endif()

target_link_libraries( instutil
   ${argumentum_util_lib}
   )
add_dependencies( instutil ${argumentum_util_lib} )
