
add_custom_command(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/fake_create_headers.cpp
   DEPENDS instutil argumentum/argparse.h
   COMMENT "Preparing library headers for publishing"

   COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/argumentum

   COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_SOURCE_DIR}/src  ${CMAKE_CURRENT_BINARY_DIR}/argumentum/inc

   COMMAND ${CMAKE_COMMAND} -E remove
      ${CMAKE_CURRENT_BINARY_DIR}/argumentum/inc/*.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/argumentum/inc/*.txt
      ${CMAKE_CURRENT_BINARY_DIR}/argumentum/inc/*_impl.h

   COMMAND sed -e "s#\.\./\.\./src#inc#g"
      ${CMAKE_CURRENT_SOURCE_DIR}/argumentum/argparse.h
      > ${CMAKE_CURRENT_BINARY_DIR}/argumentum/argparse.h

   COMMAND ${CMAKE_COMMAND} -E echo 'int main(){ return 0\; }' > fake_create_headers.cpp
   )

add_executable(fake_create_headers
   ${CMAKE_CURRENT_BINARY_DIR}/fake_create_headers.cpp
   )

install(DIRECTORY ${CMAKE_BINARY_DIR}/include/argumentum
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

