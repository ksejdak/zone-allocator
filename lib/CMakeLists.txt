message(STATUS "Configuring '${CMAKE_CURRENT_SOURCE_DIR}/version.hpp'")
configure_file(version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp)

add_library(liballocator
    allocator.cpp
    group.cpp
    page_allocator.cpp
    page.cpp
    region_info.cpp
    zone.cpp
    zone_allocator.cpp
)

target_include_directories(liballocator
    PUBLIC include
    PRIVATE .
)

target_compile_features(liballocator
    PRIVATE cxx_std_17
)
