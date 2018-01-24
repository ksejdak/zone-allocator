# Required cmake version.
cmake_minimum_required(VERSION 3.10)

# Required C++ version.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

include_directories(include)

add_subdirectory(test)
add_subdirectory(src)
