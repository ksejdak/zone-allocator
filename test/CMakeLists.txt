project(zoneallocator_tests CXX)

add_executable(zoneallocator_tests
    main.cpp
    test_page_allocator.cpp
    test_page.cpp
    test_region_info.cpp
    utils.h
)

target_compile_features(zoneallocator_tests PRIVATE cxx_std_17)

# Link with library private implementation for testing.
get_target_property(ZONEALLOCATOR_INCLUDES zoneallocator INCLUDE_DIRECTORIES)
target_include_directories(zoneallocator_tests PRIVATE ${ZONEALLOCATOR_INCLUDES})

target_link_libraries(zoneallocator_tests PUBLIC catch2
                                          PRIVATE zoneallocator
)

# Test definitions.
add_test(UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zoneallocator_tests -r xml -d yes --order lex "")
add_test(IntegrationTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/zoneallocator_tests [.][integration] -r xml -d yes --order lex "")
