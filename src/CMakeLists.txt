
add_library(${ARGUMENTUM_STATIC_NAME} STATIC
   argparser.cpp
   )
if( ARGUMENTUM_PEDANTIC )
   target_compile_options(${ARGUMENTUM_STATIC_NAME} PRIVATE 
      $<$<CXX_COMPILER_ID:GNU>:-Werror -Wall>
      $<$<CXX_COMPILER_ID:MSVC>:/WX /permissive- /Za>
      )
endif()

install( TARGETS ${ARGUMENTUM_STATIC_NAME}
   ARCHIVE DESTINATION lib
   LIBRARY DESTINATION lib
   )
