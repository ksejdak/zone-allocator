#!/bin/bash

conan install .. --build missing -pr gcc-9 -s build_type=Debug
cmake .. -DPLATFORM=linux
make
