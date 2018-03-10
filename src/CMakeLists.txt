add_library(zoneallocator
    allocator.cpp
    page_allocator.cpp
    page.cpp
    region_info.cpp
    zone_allocator.cpp
)

target_compile_features(zoneallocator PUBLIC cxx_std_17)

target_include_directories(zoneallocator
    PUBLIC ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
