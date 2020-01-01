
file( GLOB copied_headers "${CMAKE_SOURCE_DIR}/src/*.h" )

add_custom_command(
   OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/fake_create_headers.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/argumentum/argparse.h
   DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/argumentum/argparse.h
      ${copied_headers}

   COMMENT "Preparing library headers for publishing"

   COMMAND ${CMAKE_COMMAND}
       -E copy
       "${CMAKE_SOURCE_DIR}/util/faketarget.cpp"
       "${CMAKE_CURRENT_BINARY_DIR}/fake_create_headers.cpp"

   COMMAND ${CMAKE_COMMAND}
       -D TOP_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
       -D P_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
       -D P_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
       -P "${CMAKE_CURRENT_SOURCE_DIR}/prepare-headers.cmake"
   )

add_executable( fake_create_headers
   ${CMAKE_CURRENT_BINARY_DIR}/fake_create_headers.cpp
   )

install( DIRECTORY ${CMAKE_BINARY_DIR}/include/argumentum
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
   FILES_MATCHING PATTERN "*.h"
   PATTERN "inc/*_impl.h" EXCLUDE
   PATTERN "inc/*.txt" EXCLUDE
   PATTERN "inc/*.cpp" EXCLUDE
   )

