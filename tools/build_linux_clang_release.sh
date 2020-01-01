#!/bin/bash

conan install .. --build missing -pr clang-9 -s build_type=Release
cmake .. -DPLATFORM=linux -DTOOLCHAIN=clang-9 -DCMAKE_BUILD_TYPE=Release
make
