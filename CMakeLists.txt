cmake_minimum_required(VERSION 3.9)
project(libzoneallocator VERSION 0.1.0 LANGUAGES CXX)

# Project options.
option(WITH_TESTS "Build unit tests" OFF)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
add_custom_target(create_dirs ALL COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR})

add_subdirectory(src)
if(WITH_TESTS)
    message(STATUS "  Building libzoneallocator with tests")
    enable_testing()
    add_subdirectory(test)
endif()
