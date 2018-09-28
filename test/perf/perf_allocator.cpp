/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2017-2018, Kuba Sejdak <kuba.sejdak@gmail.com>
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///
/// 1. Redistributions of source code must retain the above copyright notice, this
///    list of conditions and the following disclaimer.
///
/// 2. Redistributions in binary form must reproduce the above copyright notice,
///    this list of conditions and the following disclaimer in the documentation
///    and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
/// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
/// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
/// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
/// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
/// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
/// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///
/////////////////////////////////////////////////////////////////////////////////////

#include <catch2/catch.hpp>

#include <test_utils.h>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <random>

#include <iostream>

#include <allocator/allocator.h>

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace memory;

struct PerfStats {
    double liballocatorAlloc = 0.0;
    double liballocatorRelease = 0.0;
    double mallocAlloc = 0.0;
    double mallocRelease = 0.0;
    double newAlloc = 0.0;
    double newRelease = 0.0;
};

static void perf_showStats(const PerfStats& stats, const char* name, bool showFirst = false)
{
    if (showFirst)
        std::printf("+--------------------------------+-------------+-------------+\n");
    std::printf("| %-30s |  allocate   |   release   |\n", name);
    std::printf("+--------------------------------+-------------+-------------+\n");
    std::printf("| %30s | %8.4f us | %8.4f us |\n", "liballocator", stats.liballocatorAlloc, stats.liballocatorRelease);
    std::printf("| %30s | %8.4f us | %8.4f us |\n", "malloc", stats.mallocAlloc, stats.mallocRelease);
    std::printf("| %30s | %8.4f us | %8.4f us |\n", "new", stats.newAlloc, stats.newRelease);
    std::printf("+--------------------------------+-------------+-------------+\n");
}

TEST_CASE("1000000x 134 bytes", "[perf][allocator]")
{
    std::string allocator;
    std::size_t allocSize = 134;
    constexpr int allocationsCount = 1000000;
    PerfStats stats{};

    // Initialize liballocator.
    std::size_t pageSize = 256;
    std::size_t pagesCount = 535;
    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);
    REQUIRE(allocator::init(std::uintptr_t(memory.get()), std::uintptr_t(memory.get() + size), pageSize));

    for (int i = 0; i < allocationsCount; ++i) {
        {
            auto startAlloc = test::currentTime();
            auto *ptr = allocator::allocate(allocSize);
            auto endAlloc = test::currentTime();

            auto startRelease = test::currentTime();
            allocator::release(ptr);
            auto endRelease = test::currentTime();

            stats.liballocatorAlloc += test::toMicroseconds(endAlloc - startAlloc);
            stats.liballocatorRelease += test::toMicroseconds(endRelease - startRelease);
        }

        {
            auto startAlloc = test::currentTime();
            auto *ptr = malloc(allocSize);
            auto endAlloc = test::currentTime();

            auto startRelease = test::currentTime();
            free(ptr);
            auto endRelease = test::currentTime();

            stats.mallocAlloc += test::toMicroseconds(endAlloc - startAlloc);
            stats.mallocRelease += test::toMicroseconds(endRelease - startRelease);
        }

        {
            auto startAlloc = test::currentTime();
            auto *ptr = new char[allocSize];
            auto endAlloc = test::currentTime();

            auto startRelease = test::currentTime();
            delete[] ptr;
            auto endRelease = test::currentTime();

            stats.newAlloc += test::toMicroseconds(endAlloc - startAlloc);
            stats.newRelease += test::toMicroseconds(endRelease - startRelease);
        }
    }

    stats.liballocatorAlloc /= double(allocationsCount);
    stats.liballocatorRelease /= double(allocationsCount);
    stats.mallocAlloc /= double(allocationsCount);
    stats.mallocRelease /= double(allocationsCount);
    stats.newAlloc /= double(allocationsCount);
    stats.newRelease /= double(allocationsCount);
    perf_showStats(stats, "1000000x 134 bytes", true);
}

TEST_CASE("1000000x 4056 bytes", "[perf][allocator]")
{
    std::string allocator;
    std::size_t allocSize = 4056;
    constexpr int allocationsCount = 1000000;
    PerfStats stats{};

    // Initialize liballocator.
    std::size_t pageSize = 256;
    std::size_t pagesCount = 535;
    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);
    REQUIRE(allocator::init(std::uintptr_t(memory.get()), std::uintptr_t(memory.get() + size), pageSize));

    for (int i = 0; i < allocationsCount; ++i) {
        {
            auto startAlloc = test::currentTime();
            auto *ptr = allocator::allocate(allocSize);
            auto endAlloc = test::currentTime();

            auto startRelease = test::currentTime();
            allocator::release(ptr);
            auto endRelease = test::currentTime();

            stats.liballocatorAlloc += test::toMicroseconds(endAlloc - startAlloc);
            stats.liballocatorRelease += test::toMicroseconds(endRelease - startRelease);
        }

        {
            auto startAlloc = test::currentTime();
            auto *ptr = malloc(allocSize);
            auto endAlloc = test::currentTime();

            auto startRelease = test::currentTime();
            free(ptr);
            auto endRelease = test::currentTime();

            stats.mallocAlloc += test::toMicroseconds(endAlloc - startAlloc);
            stats.mallocRelease += test::toMicroseconds(endRelease - startRelease);
        }

        {
            auto startAlloc = test::currentTime();
            auto *ptr = new char[allocSize];
            auto endAlloc = test::currentTime();

            auto startRelease = test::currentTime();
            delete[] ptr;
            auto endRelease = test::currentTime();

            stats.newAlloc += test::toMicroseconds(endAlloc - startAlloc);
            stats.newRelease += test::toMicroseconds(endRelease - startRelease);
        }
    }

    stats.liballocatorAlloc /= double(allocationsCount);
    stats.liballocatorRelease /= double(allocationsCount);
    stats.mallocAlloc /= double(allocationsCount);
    stats.mallocRelease /= double(allocationsCount);
    stats.newAlloc /= double(allocationsCount);
    stats.newRelease /= double(allocationsCount);
    perf_showStats(stats, "1000000x 4056 bytes");
}

TEST_CASE("2000x random number of bytes", "[perf][allocator]")
{
    std::string allocator;
    constexpr std::size_t allocationsCount = 2000;
    std::array<char*, allocationsCount> liballocatorMem{};
    std::array<char*, allocationsCount> mallocMem{};
    std::array<char*, allocationsCount> newMem{};
    PerfStats stats{};

    // Initialize liballocator.
    std::size_t pageSize = 256;
    std::size_t pagesCount = 4096;
    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);
    REQUIRE(memory != nullptr);
    REQUIRE(allocator::init(std::uintptr_t(memory.get()), std::uintptr_t(memory.get() + size), pageSize));

    auto maxAllocSize = pageSize;

    // Initialize random number generator.
    std::random_device randomDevice;
    std::mt19937 randomGenerator(randomDevice());
    std::uniform_int_distribution<std::size_t> distribution(0, maxAllocSize);

    for (std::size_t i = 0; i < allocationsCount; ++i) {
        std::size_t allocSize = distribution(randomGenerator);

        {
            auto startAlloc = test::currentTime();
            auto *ptr = allocator::allocate(allocSize);
            auto endAlloc = test::currentTime();

            if (ptr == nullptr) {
                --i;
                continue;
            }

            liballocatorMem.at(i) = reinterpret_cast<char*>(ptr);
            stats.liballocatorAlloc += test::toMicroseconds(endAlloc - startAlloc);
        }

        {
            auto startAlloc = test::currentTime();
            auto *ptr = malloc(allocSize);
            auto endAlloc = test::currentTime();

            if (ptr == nullptr) {
                allocator::release(liballocatorMem.at(i));
                --i;
                continue;
            }

            mallocMem.at(i) = reinterpret_cast<char*>(ptr);
            stats.mallocAlloc += test::toMicroseconds(endAlloc - startAlloc);
        }

        {
            try {
                auto startAlloc = test::currentTime();
                auto *ptr = new char[allocSize];
                auto endAlloc = test::currentTime();

                newMem.at(i) = ptr;
                stats.newAlloc += test::toMicroseconds(endAlloc - startAlloc);
            }
            catch (std::bad_alloc& exc) {
                allocator::release(liballocatorMem.at(i));
                free(mallocMem.at(i));
                --i;
                continue;
            }
        }
    }

    for (std::size_t i = 0; i < allocationsCount; ++i) {
        {
            auto startRelease = test::currentTime();
            allocator::release(liballocatorMem.at(i));
            auto endRelease = test::currentTime();

            stats.liballocatorRelease += test::toMicroseconds(endRelease - startRelease);
        }

        {
            auto startRelease = test::currentTime();
            free(mallocMem.at(i));
            auto endRelease = test::currentTime();

            stats.mallocRelease += test::toMicroseconds(endRelease - startRelease);
        }

        {
            auto startRelease = test::currentTime();
            delete[] newMem.at(i);
            auto endRelease = test::currentTime();

            stats.newRelease += test::toMicroseconds(endRelease - startRelease);
        }
    }

    stats.liballocatorAlloc /= double(allocationsCount);
    stats.liballocatorRelease /= double(allocationsCount);
    stats.mallocAlloc /= double(allocationsCount);
    stats.mallocRelease /= double(allocationsCount);
    stats.newAlloc /= double(allocationsCount);
    stats.newRelease /= double(allocationsCount);
    perf_showStats(stats, "2000x random number of bytes");
}