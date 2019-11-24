cmake_minimum_required(VERSION 3.1)

project(ArgParser)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
add_subdirectory(example)


include(cmake/googletest/googletest.cmake)
fetch_googletest(
    ${PROJECT_SOURCE_DIR}/cmake/googletest
    ${PROJECT_BINARY_DIR}/googletest
    )

enable_testing()
add_subdirectory(test)
add_subdirectory(test/staticlib)
add_subdirectory(test/headerlib)
