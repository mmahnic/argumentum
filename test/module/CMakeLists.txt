
if ( BUILD_MODULE )
   find_package( GTest REQUIRED )
   find_package( Threads REQUIRED )

   include_directories(${GTEST_INCLUDE_DIRS})

   add_executable( moduleTest
      runtest.cpp
      basic_command_t.cpp
      )
   set_property(TARGET moduleTest PROPERTY CXX_STANDARD 20)
   target_compile_options( moduleTest PRIVATE
      $<$<CXX_COMPILER_ID:MSVC>:/experimental:module>
      )

   target_link_libraries( moduleTest

      ${GTEST_LIBRARIES}
      ${CMAKE_THREAD_LIBS_INIT}
      ${ARGUMENTUM_MODULE_NAME}
      )
   add_dependencies( moduleTest ${ARGUMENTUM_MODULE_NAME} )

   add_test(
      NAME
      staticlib
      COMMAND
      ${CMAKE_BINARY_DIR}/test/staticlib/moduleTest
      )
endif()
