#!/bin/bash

conan install .. --build missing -pr gcc-9 -s build_type=Release
cmake .. -DPLATFORM=linux -DCMAKE_BUILD_TYPE=Release
make
