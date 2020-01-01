/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2017-2020, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <allocator/region.hpp>
#include <page_allocator.hpp>
#include <test_utils.hpp>

#include <catch2/catch.hpp>

#include <array>
#include <chrono>
#include <cstddef>
#include <random>

namespace memory {

TEST_CASE("PageAllocator integration tests (long-term)", "[integration][page_allocator]")
{
    using namespace std::chrono_literals;
    constexpr auto cTestDuration = 30min;
    constexpr int cAllocationsCount = 100;

    constexpr std::size_t cPageSize = 256;
    PageAllocator pageAllocator;

    constexpr std::size_t cPagesCount1 = 535;
    constexpr std::size_t cPagesCount2 = 87;
    constexpr std::size_t cPagesCount3 = 4;
    auto size1 = cPageSize * cPagesCount1;
    auto size2 = cPageSize * cPagesCount2;
    auto size3 = cPageSize * cPagesCount3;
    auto memory1 = test::alignedAlloc(cPageSize, size1);
    auto memory2 = test::alignedAlloc(cPageSize, size2);
    auto memory3 = test::alignedAlloc(cPageSize, size3);

    // clang-format off
    std::array<Region, 4> regions = {{
        {std::uintptr_t(memory1.get()), size1},
        {std::uintptr_t(memory2.get()), size2},
        {std::uintptr_t(memory3.get()), size3},
        {0,                             0}
    }};
    // clang-format on

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));
    auto freePagesCount = pageAllocator.getStats().freePagesCount;
    auto maxAllocSize = freePagesCount / 4;

    // Initialize random number generator.
    std::random_device randomDevice;
    std::mt19937 randomGenerator(randomDevice());
    std::uniform_int_distribution<std::size_t> distribution(0, maxAllocSize);

    std::array<Page*, cAllocationsCount> pages{};

    for (auto start = test::currentTime(); !test::timeElapsed(start, cTestDuration);) {
        pages.fill(nullptr);

        // Allocate pages.
        for (auto*& page : pages) {
            auto n = distribution(randomGenerator);
            page = pageAllocator.allocate(n);
        }

        // Release pages.
        for (auto* page : pages)
            pageAllocator.release(page);

        auto stats = pageAllocator.getStats();
        REQUIRE(stats.totalMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.effectiveMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.userMemorySize == stats.effectiveMemorySize - (stats.pageSize * stats.reservedPagesCount));
        REQUIRE(stats.freeMemorySize == (cPageSize * (stats.totalPagesCount - stats.reservedPagesCount)));
        REQUIRE(stats.pageSize == cPageSize);
        REQUIRE(stats.totalPagesCount == (cPagesCount1 + cPagesCount2 + cPagesCount3));
        REQUIRE(stats.reservedPagesCount == 79);
        REQUIRE(stats.freePagesCount == (stats.totalPagesCount - stats.reservedPagesCount));
        REQUIRE(stats.freePagesCount == freePagesCount);
    }
}

} // namespace memory
