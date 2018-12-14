
# find_package (GTest REQUIRED)
# link_directories( ${CMAKE_BINARY_DIR}/3rdparty/gtest )
# link_directories( ${CMAKE_BINARY_DIR}/3rdparty/gtest )

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

set(PARSER_SOURCES
   ../src/argparser.cpp
   )

add_executable( runUnitTests
   runtest.cpp
   argparser_t.cpp
   ${PARSER_SOURCES}
   )
target_link_libraries( runUnitTests ${GTEST_LIBRARY} Threads::Threads )

