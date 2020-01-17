[![Test Linux](https://github.com/kubasejdak/liballocator/workflows/Test%20Linux/badge.svg)](https://github.com/kubasejdak/liballocator/actions?query=workflow%3A%22Test+Linux%22)

[![Quality](https://github.com/kubasejdak/liballocator/workflows/Quality/badge.svg)](https://github.com/kubasejdak/liballocator/actions?query=workflow%3A%22Quality%22)
[![CodeFactor Grade](https://img.shields.io/codefactor/grade/github/kubasejdak/liballocator)](https://www.codefactor.io/repository/github/kubasejdak/liballocator)
[![codecov.io](http://codecov.io/github/kubasejdak/liballocator/coverage.svg?branch=master)](http://codecov.io/github/kubasejdak/liballocator?branch=master)

[![Github Releases](https://img.shields.io/github/release/kubasejdak/liballocator.svg)](https://github.com/kubasejdak/liballocator/releases)
[![License](https://img.shields.io/badge/License-BSD%202--Clause-orange.svg)](https://opensource.org/licenses/BSD-2-Clause)

# liballocator

The purpose of this project is to provide a convenient and fairly robust memory allocator for the embedded environments in C++.
It is based on the FreeBSD's [UMA (Universal Memory Allocator)](https://www.freebsd.org/cgi/man.cgi?query=uma&sektion=9&manpath=freebsd-release-ports)
called the zone allocator and some concepts taken from [SLAB allocator](https://en.wikipedia.org/wiki/Slab_allocation).

## Requirements

Embedded systems have very specific requirements regarding memory allocation. Here is the list of issues/features, that liballocator is trying to address:

* physical memory can be split into multiple continuous spaces,
* boundaries of physical memory space can be specified either by linker script or by programmer in code,
* allocated memory chunks should be aligned to it's size,
* client should be able to overload operator new() to use liballocator,
* allocator should be usable in systems with MMU, MPU or without memory protection,
* there should be low bookkeeping overhead,
* memory shouldn't be subject to fragmentation.

## Overview

liballocator consists of two parts:

* page allocator - responsible for managing and allocation of the physical pages. This module is aware of the
  number of continuous regions (SRAM, DDR RAM, etc).
* zone allocator - responsible for allocation of the size-aligned memory chunks. This module makes use of the page allocator
  to create the zones containing chunks of the same size.

## Requirements

* CMake v3.13.4+,
* GCC/Clang with C++17 support,
* Linux/macOS (not tested on Windows).

## Usage

To use liballocator in your CMake project all you need to do is:

* Download this repository into your project (IMPORTANT: liballocator contains git submodules):
```
git clone --recurse-submodules git@github.com:kubasejdak/liballocator.git   # Since Git 2.13
git clone --recursive git@github.com:kubasejdak/liballocator.git            # Before Git 2.13
```

* Add it to CMake with:
```
add_subdirectory(liballocator)
```

* Link liballocator with your target:
```
target_link_libraries(<YOUR_TARGET> PUBLIC liballocator)
```

If you use concepts of "Modern CMake", then all necessary flags and include paths to build and use liballocator will be automatically propagated.
Check out this example project with STM32F4DISCOVERY: [liballocator-demo](https://github.com/kubasejdak/liballocator-demo).

## Performance

Tests were performed on macOS Mojave 10.14, Macbook Pro (2,9 GHz Intel Core i5, 8 GB 2133 MHz LPDDR3).

NOTE: liballocator currently does not contain any optimizations, assembly or caches. Performance tests do not
take into account the design goals of any used allocator. Results shown below are only for illustrative purposes
and should not be used as a reference of any kind.

##### Allocate 1000000x 134 bytes
| Allocator | Allocate (g++ 8.2.0) | Release (g++ 8.2.0) | Allocate (clang++ 7.0.0) | Release (clang++ 7.0.0)
| :---: | :---: | :---: | :---: | :---: |
| liballocator | 3.0761 us | 3.1229 us | 2.2062 us | 2.2090 us |
| malloc | 0.0303 us | 0.0296 us | 0.0008 us | 0.0010 us |
| new | 0.0833 us | 0.0733 us | 0.0008 us | 0.0009 us |

##### Allocate 1000000x 4056 bytes
| Allocator | Allocate (g++ 8.2.0) | Release (g++ 8.2.0) | Allocate (clang++ 7.0.0) | Release (clang++ 7.0.0)
| :---: | :---: | :---: | :---: | :---: |
| liballocator | 2.9269 us | 3.0897 us | 2.0906 us | 2.1194 us |
| malloc | 0.0300 us | 0.0297 us | 0.0008 us | 0.0006 us |
| new | 0.0832 us | 0.0781 us | 0.0012 us | 0.0008 us |

##### Allocate 2000x random number of bytes
| Allocator | Allocate (g++ 8.2.0) | Release (g++ 8.2.0) | Allocate (clang++ 7.0.0) | Release (clang++ 7.0.0)
| :---: | :---: | :---: | :---: | :---: |
| liballocator | 16.0200 us | 10.8960 us | 14.8775 us | 9.9780 us |
| malloc | 0.1600 us | 0.2660 us | 0.0600 us | 0.0850 us |
| new | 0.0980 us | 0.2055 us | 0.0020 us | 0.0680 us |