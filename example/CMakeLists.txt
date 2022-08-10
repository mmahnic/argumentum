
include_directories( ../include )
set( argumentum_example_lib ${ARGUMENTUM_INTERNAL_NAME} )

# The library is currently header-only.
set(PARSER_SOURCES
   # ../src/argparser.cpp
   )

add_executable( basic
   basic.cpp
   )
target_link_libraries( basic
   ${argumentum_example_lib}
   )
add_dependencies( basic ${argumentum_example_lib} )

add_executable( basic_action
   basic_action.cpp
   )
target_link_libraries( basic_action
   ${argumentum_example_lib}
   )
add_dependencies( basic_action ${argumentum_example_lib} )

add_executable( basic_struct
   basic_struct.cpp
   )
target_link_libraries( basic_struct
   ${argumentum_example_lib}
   )
add_dependencies( basic_struct ${argumentum_example_lib} )

add_executable( basic_command
   basic_command.cpp
   )
target_link_libraries( basic_command
   ${argumentum_example_lib}
   )
add_dependencies( basic_command ${argumentum_example_lib} )

add_executable( example1
   example1.cpp
   )
target_link_libraries( example1
   ${argumentum_example_lib}
   )
add_dependencies( example1 ${argumentum_example_lib} )

add_executable( example2
   example2.cpp
   )
target_link_libraries( example2
   ${argumentum_example_lib}
   )
add_dependencies( example2 ${argumentum_example_lib} )


if ( BUILD_MODULE )
   add_executable( basic_command_modules
      basic_command_modules.cpp
      )

   set_property(TARGET basic_command_modules PROPERTY CXX_STANDARD 20)
   target_compile_options( basic_command_modules PRIVATE
      $<$<CXX_COMPILER_ID:MSVC>:/experimental:module>
      )

   target_link_libraries( basic_command_modules
      ${ARGUMENTUM_MODULE_NAME}
      )
   add_dependencies( basic_command_modules ${ARGUMENTUM_MODULE_NAME} )
endif()
