cmake_minimum_required(VERSION 3.1)

project(ArgParser)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(3rdparty/gtest)
add_subdirectory(src)
add_subdirectory(test)
