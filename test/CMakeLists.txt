
include_directories( ../include )
set ( argumentum_test_lib ${ARGUMENTUM_INTERNAL_NAME} )

find_package( GTest REQUIRED )
find_package( Threads REQUIRED )

add_executable( runUnitTests
   runtest.cpp
   testutil.cpp
   argparser_t.cpp
   argumentconfig_t.cpp
   number_t.cpp
   negativenumber_t.cpp
   convert_t.cpp
   group_t.cpp
   command_t.cpp
   action_t.cpp
   help_t.cpp
   commandhelp_t.cpp
   filesystemarguments_t.cpp
   argumentstream_t.cpp
   optionfactory_t.cpp
   value_t.cpp
   )

target_link_libraries( runUnitTests 
   ${GTEST_LIBRARIES}
   ${CMAKE_THREAD_LIBS_INIT}
   ${argumentum_test_lib}
   stdc++fs
   )
add_dependencies( runUnitTests ${argumentum_test_lib} )

add_executable( utilityTests
   runtest.cpp
   testutil.cpp
   testutil_t.cpp
   )

target_link_libraries( utilityTests 
   ${GTEST_LIBRARIES}
   ${CMAKE_THREAD_LIBS_INIT}
   ${argumentum_test_lib}
   )
add_dependencies( utilityTests ${argumentum_test_lib} )

add_test(
  NAME
    utility
  COMMAND
    ${CMAKE_BINARY_DIR}/test/utilityTests
)

add_test(
  NAME
    parser
  COMMAND
    ${CMAKE_BINARY_DIR}/test/runUnitTests
)

