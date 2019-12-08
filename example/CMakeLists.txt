
include_directories( ../include )

# The library is currently header-only.
set(PARSER_SOURCES
   # ../src/argparser.cpp
   )

add_executable( basic
   basic.cpp
   )
target_link_libraries( basic
   ${CMAKE_BINARY_DIR}/src/libargparse-s.a
   )

add_executable( basic_action
   basic_action.cpp
   )
target_link_libraries( basic_action
   ${CMAKE_BINARY_DIR}/src/libargparse-s.a
   )

add_executable( basic_struct
   basic_struct.cpp
   )
target_link_libraries( basic_struct
   ${CMAKE_BINARY_DIR}/src/libargparse-s.a
   )

add_executable( basic_command
   basic_command.cpp
   )
target_link_libraries( basic_command
   ${CMAKE_BINARY_DIR}/src/libargparse-s.a
   )

add_executable( example1
   example1.cpp
   )
target_link_libraries( example1
   ${CMAKE_BINARY_DIR}/src/libargparse-s.a
   )

add_executable( example2
   example2.cpp
   )
target_link_libraries( example2
   ${CMAKE_BINARY_DIR}/src/libargparse-s.a
   )

