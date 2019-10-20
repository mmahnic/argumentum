
include_directories( ../include )

# The library is currently header-only.
set(PARSER_SOURCES
   # ../src/argparser.cpp
   )

add_executable( basic
   basic.cpp
   )

add_executable( example1
   example1.cpp
   )

add_executable( example2
   example2.cpp
   )

