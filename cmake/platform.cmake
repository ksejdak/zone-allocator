include(FetchContent)
FetchContent_Declare(
    platform
    GIT_REPOSITORY  https://github.com/kubasejdak/platform.git
    GIT_TAG         master
)

FetchContent_GetProperties(platform)
if (NOT platform_POPULATED)
    FetchContent_Populate(platform)
endif ()
