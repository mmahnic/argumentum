
add_library(${CPPARGPARSE_STATIC_NAME} STATIC
   argparser.cpp
   )
if( CPPARGPARSE_PEDANTIC )
   target_compile_options(${CPPARGPARSE_STATIC_NAME} PRIVATE 
      $<$<CXX_COMPILER_ID:GNU>:-Werror -Wall>
      $<$<CXX_COMPILER_ID:MSVC>:/WX /permissive- /Za>
      )
endif()

if (CPPARGPARSE_INSTALL)
   install( TARGETS ${CPPARGPARSE_STATIC_NAME}
      ARCHIVE DESTINATION lib
      LIBRARY DESTINATION lib
      )
endif()
