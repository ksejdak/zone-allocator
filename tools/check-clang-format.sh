#!/bin/bash
# $1... - paths to look for source files

echo "Running clang-format..."

for SRC_PATH in "$@"; do
    ALL_SOURCES=$(find "${SRC_PATH}" -iname "*.h" -o -iname "*.hpp" -o -iname "*.c" -o -iname "*.cpp")
    TO_CHECK_SOURCES=$(echo "${ALL_SOURCES}" | sed '/FreeRTOSConfig.h/d')

    echo "${TO_CHECK_SOURCES}" | xargs clang-format -style=file -fallback-style=none -i
done

echo "Done."
exit 0
