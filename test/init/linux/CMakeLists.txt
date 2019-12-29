add_library(platform-init EXCLUDE_FROM_ALL
    platformInit.cpp
)

target_include_directories(platform-init
    PUBLIC ..
)
