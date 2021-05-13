
include_directories( ../include )
set ( argumentum_test_lib ${ARGUMENTUM_INTERNAL_NAME} )

find_package( GTest REQUIRED )
find_package( Threads REQUIRED )

include_directories(${GTEST_INCLUDE_DIRS})

add_executable( argumentumTests
   runtest.cpp
   testutil.cpp

   # argparser_depr_t.cpp
   action_t.cpp
   argparser_t.cpp
   argumentstream_t.cpp
   command_t.cpp
   commandhelp_t.cpp
   convert_t.cpp
   filesystemarguments_t.cpp
   forwardparam_t.cpp
   group_t.cpp
   help_t.cpp
   metavar_t.cpp
   negativenumber_t.cpp
   number_t.cpp
   optionfactory_t.cpp
   parameterconfig_t.cpp
   parserconfig_t.cpp
   value_t.cpp
   )

target_link_libraries( argumentumTests
   ${GTEST_LIBRARIES}
   ${CMAKE_THREAD_LIBS_INIT}
   ${argumentum_test_lib}
   )

if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
   target_link_libraries( argumentumTests  stdc++fs )
endif()

add_dependencies( argumentumTests ${argumentum_test_lib} )

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
    ${CMAKE_BINARY_DIR}/test/argumentumTests
)

