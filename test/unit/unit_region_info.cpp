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
#include <region_info.hpp>

#include <catch2/catch.hpp>

#include <array>
#include <cstddef>
#include <cstring>

namespace memory {

TEST_CASE("RegionInfo structure is properly cleared", "[unit][region_info]")
{
    RegionInfo regionInfo{};
    constexpr int cPattern = 0x5a;
    std::memset(&regionInfo, cPattern, sizeof(RegionInfo));

    clearRegionInfo(regionInfo);
    REQUIRE(regionInfo.start == 0);
    REQUIRE(regionInfo.end == 0);
    REQUIRE(regionInfo.alignedStart == 0);
    REQUIRE(regionInfo.alignedEnd == 0);
    REQUIRE(regionInfo.pageCount == 0);
    REQUIRE(regionInfo.size == 0);
    REQUIRE(regionInfo.alignedSize == 0);
    REQUIRE(regionInfo.firstPage == nullptr);
    REQUIRE(regionInfo.lastPage == nullptr);
}

TEST_CASE("Aligned start address is properly computed", "[unit][region_info]")
{
    constexpr std::size_t cPageSize = 4096;

    SECTION("Already start-aligned region")
    {
        alignas(cPageSize) std::array<std::byte, cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory)), memory.size()};

        auto start = detail::alignedStart(region, cPageSize);
        REQUIRE(start);
        REQUIRE(*start == std::uintptr_t(std::begin(memory)));
    }

    SECTION("Not start-aligned region and aligned start is within region boundaries")
    {
        constexpr std::size_t cOffset = 15;
        alignas(cPageSize) std::array<std::byte, 2 * cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory) + cOffset), memory.size() - cOffset};

        auto start = detail::alignedStart(region, cPageSize);
        REQUIRE(start);
        REQUIRE(*start == std::uintptr_t(std::begin(memory) + cPageSize));
    }

    SECTION("Not start-aligned region and aligned start is outside region boundaries")
    {
        constexpr std::size_t cOffset = 15;
        alignas(cPageSize) std::array<std::byte, cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory) + cOffset), memory.size() - 2 * cOffset};

        auto start = detail::alignedStart(region, cPageSize);
        REQUIRE(!start);
    }
}

TEST_CASE("Aligned end address is properly computed", "[unit][region_info]")
{
    constexpr std::size_t cPageSize = 4096;

    SECTION("Already end-aligned region")
    {
        alignas(cPageSize) std::array<std::byte, cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory)), memory.size()};

        auto end = detail::alignedEnd(region, cPageSize);
        REQUIRE(end);
        REQUIRE(*end == std::uintptr_t(std::end(memory)));
    }

    SECTION("Not end-aligned region and aligned end is within region boundaries")
    {
        constexpr std::size_t cOffset = 15;
        alignas(cPageSize) std::array<std::byte, 2 * cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory)), memory.size() - cOffset};

        auto end = detail::alignedEnd(region, cPageSize);
        REQUIRE(end);
        REQUIRE(*end == std::uintptr_t(std::begin(memory) + cPageSize));
    }

    SECTION("Not end-aligned region and aligned end is outside region boundaries")
    {
        constexpr std::size_t cOffset = 15;
        alignas(cPageSize) std::array<std::byte, cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory)) + cOffset, memory.size() - 2 * cOffset};

        auto end = detail::alignedEnd(region, cPageSize);
        REQUIRE(!end);
    }
}

TEST_CASE("RegionInfo is properly initialized", "[unit][region_info]")
{
    constexpr std::size_t cPageSize = 4096;
    RegionInfo regionInfo{};
    clearRegionInfo(regionInfo);

    SECTION("Region smaller than one page")
    {
        alignas(cPageSize) std::array<std::byte, cPageSize - 1> memory{};
        Region region = {std::uintptr_t(std::begin(memory)), memory.size()};

        bool result = initRegionInfo(regionInfo, region, cPageSize);
        REQUIRE(!result);
    }

    SECTION("Fully aligned region, lays on 1 page")
    {
        constexpr std::size_t cPageCount = 1;
        alignas(cPageSize) std::array<std::byte, cPageCount * cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory)), memory.size()};

        bool result = initRegionInfo(regionInfo, region, cPageSize);
        REQUIRE(result);
        REQUIRE(regionInfo.start == region.address);
        REQUIRE(regionInfo.end == region.address + region.size);
        REQUIRE(regionInfo.alignedStart == region.address);
        REQUIRE(regionInfo.alignedEnd == region.address + region.size);
        REQUIRE(regionInfo.pageCount == cPageCount);
        REQUIRE(regionInfo.size == region.size);
        REQUIRE(regionInfo.alignedSize == region.size);
        REQUIRE(regionInfo.firstPage == nullptr);
        REQUIRE(regionInfo.lastPage == nullptr);
    }

    SECTION("Fully aligned region, lays on 5 pages")
    {
        constexpr std::size_t cPageCount = 5;
        alignas(cPageSize) std::array<std::byte, cPageCount * cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory)), memory.size()};

        bool result = initRegionInfo(regionInfo, region, cPageSize);
        REQUIRE(result);
        REQUIRE(regionInfo.start == region.address);
        REQUIRE(regionInfo.end == region.address + region.size);
        REQUIRE(regionInfo.alignedStart == region.address);
        REQUIRE(regionInfo.alignedEnd == region.address + region.size);
        REQUIRE(regionInfo.pageCount == cPageCount);
        REQUIRE(regionInfo.size == region.size);
        REQUIRE(regionInfo.alignedSize == region.size);
        REQUIRE(regionInfo.firstPage == nullptr);
        REQUIRE(regionInfo.lastPage == nullptr);
    }

    SECTION("Start-aligned region, lays on 1 page")
    {
        constexpr std::size_t cPageCount = 1;
        constexpr std::size_t cOffset = 15;
        alignas(cPageSize) std::array<std::byte, cPageCount * cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory)), memory.size() - cOffset};

        bool result = initRegionInfo(regionInfo, region, cPageSize);
        REQUIRE(!result);
    }

    SECTION("Start-aligned region, lays on 2 pages")
    {
        constexpr std::size_t cPageCount = 2;
        constexpr std::size_t cOffset = 15;
        alignas(cPageSize) std::array<std::byte, cPageCount * cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory)), memory.size() - cOffset};

        bool result = initRegionInfo(regionInfo, region, cPageSize);
        REQUIRE(result);
        REQUIRE(regionInfo.start == region.address);
        REQUIRE(regionInfo.end == region.address + region.size);
        REQUIRE(regionInfo.alignedStart == region.address);
        REQUIRE(regionInfo.alignedEnd == region.address + (cPageCount - 1) * cPageSize);
        REQUIRE(regionInfo.pageCount == (cPageCount - 1));
        REQUIRE(regionInfo.size == region.size);
        REQUIRE(regionInfo.alignedSize == (cPageCount - 1) * cPageSize);
        REQUIRE(regionInfo.firstPage == nullptr);
        REQUIRE(regionInfo.lastPage == nullptr);
    }

    SECTION("Start-aligned region, lays on 5 pages")
    {
        constexpr std::size_t cPageCount = 5;
        constexpr std::size_t cOffset = 15;
        alignas(cPageSize) std::array<std::byte, cPageCount * cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory)), memory.size() - cOffset};

        bool result = initRegionInfo(regionInfo, region, cPageSize);
        REQUIRE(result);
        REQUIRE(regionInfo.start == region.address);
        REQUIRE(regionInfo.end == region.address + region.size);
        REQUIRE(regionInfo.alignedStart == region.address);
        REQUIRE(regionInfo.alignedEnd == region.address + (cPageCount - 1) * cPageSize);
        REQUIRE(regionInfo.pageCount == (cPageCount - 1));
        REQUIRE(regionInfo.size == region.size);
        REQUIRE(regionInfo.alignedSize == (cPageCount - 1) * cPageSize);
        REQUIRE(regionInfo.firstPage == nullptr);
        REQUIRE(regionInfo.lastPage == nullptr);
    }

    SECTION("End-aligned region, lays on 1 page")
    {
        constexpr std::size_t cPageCount = 1;
        constexpr std::size_t cOffset = 15;
        alignas(cPageSize) std::array<std::byte, cPageCount * cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory) + cOffset), memory.size() - cOffset};

        bool result = initRegionInfo(regionInfo, region, cPageSize);
        REQUIRE(!result);
    }

    SECTION("End-aligned region, lays on 2 pages")
    {
        constexpr std::size_t cPageCount = 2;
        constexpr std::size_t cOffset = 15;
        alignas(cPageSize) std::array<std::byte, cPageCount * cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory) + cOffset), memory.size() - cOffset};

        bool result = initRegionInfo(regionInfo, region, cPageSize);
        REQUIRE(result);
        REQUIRE(regionInfo.start == region.address);
        REQUIRE(regionInfo.end == region.address + region.size);
        REQUIRE(regionInfo.alignedStart == std::uintptr_t(std::begin(memory) + cPageSize));
        REQUIRE(regionInfo.alignedEnd == region.address + region.size);
        REQUIRE(regionInfo.pageCount == (cPageCount - 1));
        REQUIRE(regionInfo.size == region.size);
        REQUIRE(regionInfo.alignedSize == (cPageCount - 1) * cPageSize);
        REQUIRE(regionInfo.firstPage == nullptr);
        REQUIRE(regionInfo.lastPage == nullptr);
    }

    SECTION("End-aligned region, lays on 5 pages")
    {
        constexpr std::size_t cPageCount = 5;
        constexpr std::size_t cOffset = 15;
        alignas(cPageSize) std::array<std::byte, cPageCount * cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory) + cOffset), memory.size() - cOffset};

        bool result = initRegionInfo(regionInfo, region, cPageSize);
        REQUIRE(result);
        REQUIRE(regionInfo.start == region.address);
        REQUIRE(regionInfo.end == region.address + region.size);
        REQUIRE(regionInfo.alignedStart == std::uintptr_t(std::begin(memory) + cPageSize));
        REQUIRE(regionInfo.alignedEnd == region.address + region.size);
        REQUIRE(regionInfo.pageCount == (cPageCount - 1));
        REQUIRE(regionInfo.size == region.size);
        REQUIRE(regionInfo.alignedSize == (cPageCount - 1) * cPageSize);
        REQUIRE(regionInfo.firstPage == nullptr);
        REQUIRE(regionInfo.lastPage == nullptr);
    }

    SECTION("Fully unaligned region, lays on 1 page")
    {
        constexpr std::size_t cPageCount = 1;
        constexpr std::size_t cOffset = 15;
        alignas(cPageSize) std::array<std::byte, cPageCount * cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory) + cOffset), memory.size() - 2 * cOffset};

        bool result = initRegionInfo(regionInfo, region, cPageSize);
        REQUIRE(!result);
    }

    SECTION("Fully unaligned region, lays on 2 pages")
    {
        constexpr std::size_t cPageCount = 2;
        constexpr std::size_t cOffset = 15;
        alignas(cPageSize) std::array<std::byte, cPageCount * cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory) + cOffset), memory.size() - 2 * cOffset};

        bool result = initRegionInfo(regionInfo, region, cPageSize);
        REQUIRE(!result);
    }

    SECTION("Fully unaligned region, lays on 5 pages")
    {
        constexpr std::size_t cPageCount = 5;
        constexpr std::size_t cOffset = 15;
        alignas(cPageSize) std::array<std::byte, cPageCount * cPageSize> memory{};
        Region region = {std::uintptr_t(std::begin(memory) + cOffset), memory.size() - 2 * cOffset};

        bool result = initRegionInfo(regionInfo, region, cPageSize);
        REQUIRE(result);
        REQUIRE(regionInfo.start == region.address);
        REQUIRE(regionInfo.end == region.address + region.size);
        REQUIRE(regionInfo.alignedStart == std::uintptr_t(std::begin(memory) + cPageSize));
        REQUIRE(regionInfo.alignedEnd == std::uintptr_t(std::end(memory) - cPageSize));
        REQUIRE(regionInfo.pageCount == (cPageCount - 2));
        REQUIRE(regionInfo.size == region.size);
        REQUIRE(regionInfo.alignedSize == (cPageCount - 2) * cPageSize);
        REQUIRE(regionInfo.firstPage == nullptr);
        REQUIRE(regionInfo.lastPage == nullptr);
    }
}

} // namespace memory
