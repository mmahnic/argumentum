
# The library is currently header-only.
set(PARSER_SOURCES
   # ../src/argparser.cpp
   )

add_executable( runUnitTests
   runtest.cpp
   argparser_t.cpp
   argparserconvert_t.cpp
   argparsergroup_t.cpp
   argparsercommand_t.cpp
   action_t.cpp
   argparserhelp_t.cpp
   helpwriter_t.cpp
   streamarguments_t.cpp
   argumentstream_t.cpp
   ${PARSER_SOURCES}
   )

target_link_libraries( runUnitTests 
   gtest_main
   )

add_test(
  NAME
    parser
  COMMAND
    ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/test/runUnitTests
)
