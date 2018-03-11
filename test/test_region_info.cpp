////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017-2018, Kuba Sejdak <kuba.sejdak@gmail.com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
////////////////////////////////////////////////////////////////////////////////////

#include <catch2/catch.hpp>

#include <region_info.h>

#include <array>
#include <cstddef>
#include <cstring>

using namespace Memory;

TEST_CASE("RegionInfo structure is properly cleared", "[region_info]")
{
    RegionInfo regionInfo;
    std::memset(&regionInfo, 0x5a, sizeof(RegionInfo));

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

TEST_CASE("Aligned start address is properly computed", "[region_info]")
{
    constexpr std::size_t pageSize = 4096;

    SECTION("Already start-aligned region")
    {
        alignas(pageSize) std::array<std::byte, pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory)), .size = memory.size()};

        auto start = detail::alignedStart(region, pageSize);
        REQUIRE(start);
        REQUIRE(*start == std::uintptr_t(std::begin(memory)));
    }

    SECTION("Not start-aligned region and aligned start is within region boundaries")
    {
        constexpr std::size_t offset = 15;
        alignas(pageSize) std::array<std::byte, 2 * pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory) + offset), .size = memory.size() - offset};

        auto start = detail::alignedStart(region, pageSize);
        REQUIRE(start);
        REQUIRE(*start == std::uintptr_t(std::begin(memory) + pageSize));
    }

    SECTION("Not start-aligned region and aligned start is outside region boundaries")
    {
        constexpr std::size_t offset = 15;
        alignas(pageSize) std::array<std::byte, pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory) + offset), .size = memory.size() - 2 * offset};

        auto start = detail::alignedStart(region, pageSize);
        REQUIRE(!start);
    }
}

TEST_CASE("Aligned end address is properly computed", "[region_info]")
{
    constexpr std::size_t pageSize = 4096;

    SECTION("Already end-aligned region")
    {
        alignas(pageSize) std::array<std::byte, pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory)), .size = memory.size()};

        auto end = detail::alignedEnd(region, pageSize);
        REQUIRE(end);
        REQUIRE(*end == std::uintptr_t(std::end(memory)));
    }

    SECTION("Not end-aligned region and aligned end is within region boundaries")
    {
        constexpr std::size_t offset = 15;
        alignas(pageSize) std::array<std::byte, 2 * pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory)), .size = memory.size() - offset};

        auto end = detail::alignedEnd(region, pageSize);
        REQUIRE(end);
        REQUIRE(*end == std::uintptr_t(std::begin(memory) + pageSize));
    }

    SECTION("Not end-aligned region and aligned end is outsude region boundaries")
    {
        constexpr std::size_t offset = 15;
        alignas(pageSize) std::array<std::byte, pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory)) + offset, .size = memory.size() - 2 * offset};

        auto end = detail::alignedEnd(region, pageSize);
        REQUIRE(!end);
    }
}

TEST_CASE("RegionInfo is properly initialized", "[region_info]")
{
    constexpr std::size_t pageSize = 4096;
    RegionInfo regionInfo;
    clearRegionInfo(regionInfo);

    SECTION("Region smaller than one page")
    {
        alignas(pageSize) std::array<std::byte, pageSize - 1> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory)), .size = memory.size()};

        bool result = initRegionInfo(regionInfo, region, pageSize);
        REQUIRE(!result);
    }

    SECTION("Fully aligned region, lays on 1 page")
    {
        constexpr std::size_t pageCount = 1;
        alignas(pageSize) std::array<std::byte, pageCount * pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory)), .size = memory.size()};

        bool result = initRegionInfo(regionInfo, region, pageSize);
        REQUIRE(result);
        REQUIRE(regionInfo.start == region.address);
        REQUIRE(regionInfo.end == region.address + region.size);
        REQUIRE(regionInfo.alignedStart == region.address);
        REQUIRE(regionInfo.alignedEnd == region.address + region.size);
        REQUIRE(regionInfo.pageCount == pageCount);
        REQUIRE(regionInfo.size == region.size);
        REQUIRE(regionInfo.alignedSize == region.size);
        REQUIRE(regionInfo.firstPage == nullptr);
        REQUIRE(regionInfo.lastPage == nullptr);
    }

    SECTION("Fully aligned region, lays on 5 pages")
    {
        constexpr std::size_t pageCount = 5;
        alignas(pageSize) std::array<std::byte, pageCount * pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory)), .size = memory.size()};

        bool result = initRegionInfo(regionInfo, region, pageSize);
        REQUIRE(result);
        REQUIRE(regionInfo.start == region.address);
        REQUIRE(regionInfo.end == region.address + region.size);
        REQUIRE(regionInfo.alignedStart == region.address);
        REQUIRE(regionInfo.alignedEnd == region.address + region.size);
        REQUIRE(regionInfo.pageCount == pageCount);
        REQUIRE(regionInfo.size == region.size);
        REQUIRE(regionInfo.alignedSize == region.size);
        REQUIRE(regionInfo.firstPage == nullptr);
        REQUIRE(regionInfo.lastPage == nullptr);
    }

    SECTION("Start-aligned region, lays on 1 page")
    {
        constexpr std::size_t pageCount = 1;
        std::size_t offset = 15;
        alignas(pageSize) std::array<std::byte, pageCount * pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory)), .size = memory.size() - offset};

        bool result = initRegionInfo(regionInfo, region, pageSize);
        REQUIRE(!result);
    }

    SECTION("Start-aligned region, lays on 2 pages")
    {
        constexpr std::size_t pageCount = 2;
        std::size_t offset = 15;
        alignas(pageSize) std::array<std::byte, pageCount * pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory)), .size = memory.size() - offset};

        bool result = initRegionInfo(regionInfo, region, pageSize);
        REQUIRE(result);
        REQUIRE(regionInfo.start == region.address);
        REQUIRE(regionInfo.end == region.address + region.size);
        REQUIRE(regionInfo.alignedStart == region.address);
        REQUIRE(regionInfo.alignedEnd == region.address + (pageCount - 1) * pageSize);
        REQUIRE(regionInfo.pageCount == (pageCount - 1));
        REQUIRE(regionInfo.size == region.size);
        REQUIRE(regionInfo.alignedSize == (pageCount - 1) * pageSize);
        REQUIRE(regionInfo.firstPage == nullptr);
        REQUIRE(regionInfo.lastPage == nullptr);
    }

    SECTION("Start-aligned region, lays on 5 pages")
    {
        constexpr std::size_t pageCount = 5;
        std::size_t offset = 15;
        alignas(pageSize) std::array<std::byte, pageCount * pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory)), .size = memory.size() - offset};

        bool result = initRegionInfo(regionInfo, region, pageSize);
        REQUIRE(result);
        REQUIRE(regionInfo.start == region.address);
        REQUIRE(regionInfo.end == region.address + region.size);
        REQUIRE(regionInfo.alignedStart == region.address);
        REQUIRE(regionInfo.alignedEnd == region.address + (pageCount - 1) * pageSize);
        REQUIRE(regionInfo.pageCount == (pageCount - 1));
        REQUIRE(regionInfo.size == region.size);
        REQUIRE(regionInfo.alignedSize == (pageCount - 1) * pageSize);
        REQUIRE(regionInfo.firstPage == nullptr);
        REQUIRE(regionInfo.lastPage == nullptr);
    }

    SECTION("End-aligned region, lays on 1 page")
    {
        constexpr std::size_t pageCount = 1;
        std::size_t offset = 15;
        alignas(pageSize) std::array<std::byte, pageCount * pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory) + offset), .size = memory.size() - offset};

        bool result = initRegionInfo(regionInfo, region, pageSize);
        REQUIRE(!result);
    }

    SECTION("End-aligned region, lays on 2 pages")
    {
        constexpr std::size_t pageCount = 2;
        std::size_t offset = 15;
        alignas(pageSize) std::array<std::byte, pageCount * pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory) + offset), .size = memory.size() - offset};

        bool result = initRegionInfo(regionInfo, region, pageSize);
        REQUIRE(result);
        REQUIRE(regionInfo.start == region.address);
        REQUIRE(regionInfo.end == region.address + region.size);
        REQUIRE(regionInfo.alignedStart == std::uintptr_t(std::begin(memory) + pageSize));
        REQUIRE(regionInfo.alignedEnd == region.address + region.size);
        REQUIRE(regionInfo.pageCount == (pageCount - 1));
        REQUIRE(regionInfo.size == region.size);
        REQUIRE(regionInfo.alignedSize == (pageCount - 1) * pageSize);
        REQUIRE(regionInfo.firstPage == nullptr);
        REQUIRE(regionInfo.lastPage == nullptr);
    }

    SECTION("End-aligned region, lays on 5 pages")
    {
        constexpr std::size_t pageCount = 5;
        std::size_t offset = 15;
        alignas(pageSize) std::array<std::byte, pageCount * pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory) + offset), .size = memory.size() - offset};

        bool result = initRegionInfo(regionInfo, region, pageSize);
        REQUIRE(result);
        REQUIRE(regionInfo.start == region.address);
        REQUIRE(regionInfo.end == region.address + region.size);
        REQUIRE(regionInfo.alignedStart == std::uintptr_t(std::begin(memory) + pageSize));
        REQUIRE(regionInfo.alignedEnd == region.address + region.size);
        REQUIRE(regionInfo.pageCount == (pageCount - 1));
        REQUIRE(regionInfo.size == region.size);
        REQUIRE(regionInfo.alignedSize == (pageCount - 1) * pageSize);
        REQUIRE(regionInfo.firstPage == nullptr);
        REQUIRE(regionInfo.lastPage == nullptr);
    }

    SECTION("Fully unaligned region, lays on 1 page")
    {
        constexpr std::size_t pageCount = 1;
        std::size_t offset = 15;
        alignas(pageSize) std::array<std::byte, pageCount * pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory) + offset), .size = memory.size() - 2 * offset};

        bool result = initRegionInfo(regionInfo, region, pageSize);
        REQUIRE(!result);
    }

    SECTION("Fully unaligned region, lays on 2 pages")
    {
        constexpr std::size_t pageCount = 2;
        std::size_t offset = 15;
        alignas(pageSize) std::array<std::byte, pageCount * pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory) + offset), .size = memory.size() - 2 * offset};

        bool result = initRegionInfo(regionInfo, region, pageSize);
        REQUIRE(!result);
    }

    SECTION("Fully unaligned region, lays on 5 pages")
    {
        constexpr std::size_t pageCount = 5;
        std::size_t offset = 15;
        alignas(pageSize) std::array<std::byte, pageCount * pageSize> memory;
        Region region = {.address = std::uintptr_t(std::begin(memory) + offset), .size = memory.size() - 2 * offset};

        bool result = initRegionInfo(regionInfo, region, pageSize);
        REQUIRE(result);
        REQUIRE(regionInfo.start == region.address);
        REQUIRE(regionInfo.end == region.address + region.size);
        REQUIRE(regionInfo.alignedStart == std::uintptr_t(std::begin(memory) + pageSize));
        REQUIRE(regionInfo.alignedEnd == std::uintptr_t(std::end(memory) - pageSize));
        REQUIRE(regionInfo.pageCount == (pageCount - 2));
        REQUIRE(regionInfo.size == region.size);
        REQUIRE(regionInfo.alignedSize == (pageCount - 2) * pageSize);
        REQUIRE(regionInfo.firstPage == nullptr);
        REQUIRE(regionInfo.lastPage == nullptr);
    }
}
