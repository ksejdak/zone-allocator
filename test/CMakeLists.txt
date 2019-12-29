add_subdirectory(init/${PLATFORM})

add_library(appliballocator-tests OBJECT EXCLUDE_FROM_ALL
    appMain.cpp
    integration/integration_page_allocator.cpp
    integration/integration_zone_allocator.cpp
    perf/perf_allocator.cpp
    unit/unit_allocator.cpp
    unit/unit_group.cpp
    unit/unit_list_node.cpp
    unit/unit_page.cpp
    unit/unit_page_allocator.cpp
    unit/unit_region_info.cpp
    unit/unit_utils.cpp
    unit/unit_zone.cpp
    unit/unit_zone_allocator.cpp
)

# Link with private implementation of library for testing.
get_target_property(ALLOCATOR_INCLUDES liballocator INCLUDE_DIRECTORIES)

target_include_directories(appliballocator-tests
    PRIVATE . ${ALLOCATOR_INCLUDES}
)

target_compile_features(appliballocator-tests
    PRIVATE cxx_std_17
)

find_package(Catch2 REQUIRED)

target_link_libraries(appliballocator-tests
    PRIVATE platform-init Catch2::Catch2 liballocator
)

# This is required to selectively run Catch2 tests using tags on platforms, where cmd line arguments are not available.
if (DEFINED TEST_TAGS)
    target_compile_definitions(appliballocator-tests
        PRIVATE TEST_TAGS="${TEST_TAGS}"
    )
endif ()
