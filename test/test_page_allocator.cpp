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

#include "utils.h"

// Make access to private members for testing.
// clang-format off
#define private     public
// clang-format on

#include <page_allocator.h>

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <map>
#include <random>
#include <vector>

using namespace Memory;

TEST_CASE("Page allocator is properly cleared", "[page_allocator]")
{
    PageAllocator pageAllocator;
    std::memset(&pageAllocator, 0x5a, sizeof(PageAllocator));

    pageAllocator.clear();
    for (const auto& region : pageAllocator.m_regionsInfo) {
        REQUIRE(region.start == 0);
        REQUIRE(region.end == 0);
        REQUIRE(region.alignedStart == 0);
        REQUIRE(region.alignedEnd == 0);
        REQUIRE(region.pageCount == 0);
        REQUIRE(region.size == 0);
        REQUIRE(region.alignedSize == 0);
        REQUIRE(region.firstPage == nullptr);
        REQUIRE(region.lastPage == nullptr);
    }
    REQUIRE(pageAllocator.m_validRegionsCount == 0);
    REQUIRE(pageAllocator.m_pageSize == 0);
    REQUIRE(pageAllocator.m_descRegionIdx == 0);
    REQUIRE(pageAllocator.m_descPagesCount == 0);
    REQUIRE(pageAllocator.m_pagesHead == nullptr);
    REQUIRE(pageAllocator.m_pagesTail == nullptr);
    for (const auto* page : pageAllocator.m_freeGroupLists)
        REQUIRE(page == nullptr);
    REQUIRE(pageAllocator.m_pagesCount == 0);
    REQUIRE(pageAllocator.m_freePagesCount == 0);
}

TEST_CASE("Pages are correctly counted", "[page_allocator]")
{
    std::size_t pageSize = 256;
    PageAllocator pageAllocator;

    SECTION("Regions: 1(1)")
    {
        std::size_t pagesCount = 1;
        auto size = pageSize * pagesCount;
        auto memory = test_alignedAlloc(pageSize, size);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory.get()), size},
            {0,                            0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_pagesCount == pagesCount);
    }

    SECTION("Regions: 1(535), 2(87), 3(4)")
    {
        std::size_t pagesCount1 = 535;
        std::size_t pagesCount2 = 87;
        std::size_t pagesCount3 = 4;
        auto size1 = pageSize * pagesCount1;
        auto size2 = pageSize * pagesCount2;
        auto size3 = pageSize * pagesCount3;
        auto memory1 = test_alignedAlloc(pageSize, size1);
        auto memory2 = test_alignedAlloc(pageSize, size2);
        auto memory3 = test_alignedAlloc(pageSize, size3);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory1.get()), size1},
            {std::uintptr_t(memory2.get()), size2},
            {std::uintptr_t(memory3.get()), size3},
            {0,                             0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_pagesCount == (pagesCount1 + pagesCount2 + pagesCount3));
    }

    SECTION("All regions have 5 pages")
    {
        std::size_t pagesCount = 5;
        auto size = pageSize * pagesCount;
        auto memory1 = test_alignedAlloc(pageSize, size);
        auto memory2 = test_alignedAlloc(pageSize, size);
        auto memory3 = test_alignedAlloc(pageSize, size);
        auto memory4 = test_alignedAlloc(pageSize, size);
        auto memory5 = test_alignedAlloc(pageSize, size);
        auto memory6 = test_alignedAlloc(pageSize, size);
        auto memory7 = test_alignedAlloc(pageSize, size);
        auto memory8 = test_alignedAlloc(pageSize, size);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory1.get()), size},
            {std::uintptr_t(memory2.get()), size},
            {std::uintptr_t(memory3.get()), size},
            {std::uintptr_t(memory4.get()), size},
            {std::uintptr_t(memory5.get()), size},
            {std::uintptr_t(memory6.get()), size},
            {std::uintptr_t(memory7.get()), size},
            {std::uintptr_t(memory8.get()), size},
            {0,                              0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_pagesCount == (pagesCount * 8));
    }
}

TEST_CASE("Region where page descriptors are stored is properly selected", "[page_allocator]")
{
    std::size_t pageSize = 256;
    PageAllocator pageAllocator;

    SECTION("Regions: 1(1)")
    {
        std::size_t pagesCount = 1;
        auto size = pageSize * pagesCount;
        auto memory = test_alignedAlloc(pageSize, size);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory.get()), size},
            {0,                            0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_descRegionIdx == 0);

    }

    SECTION("Regions: 1(535), 2(87), 3(4)")
    {
        std::size_t pagesCount1 = 535;
        std::size_t pagesCount2 = 87;
        std::size_t pagesCount3 = 4;
        auto size1 = pageSize * pagesCount1;
        auto size2 = pageSize * pagesCount2;
        auto size3 = pageSize * pagesCount3;
        auto memory1 = test_alignedAlloc(pageSize, size1);
        auto memory2 = test_alignedAlloc(pageSize, size2);
        auto memory3 = test_alignedAlloc(pageSize, size3);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory1.get()), size1},
            {std::uintptr_t(memory2.get()), size2},
            {std::uintptr_t(memory3.get()), size3},
            {0,                             0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_descRegionIdx == 1);
    }

    SECTION("All regions have 5 pages")
    {
        std::size_t pagesCount = 5;
        auto size = pageSize * pagesCount;
        auto memory1 = test_alignedAlloc(pageSize, size);
        auto memory2 = test_alignedAlloc(pageSize, size);
        auto memory3 = test_alignedAlloc(pageSize, size);
        auto memory4 = test_alignedAlloc(pageSize, size);
        auto memory5 = test_alignedAlloc(pageSize, size);
        auto memory6 = test_alignedAlloc(pageSize, size);
        auto memory7 = test_alignedAlloc(pageSize, size);
        auto memory8 = test_alignedAlloc(pageSize, size);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory1.get()), size},
            {std::uintptr_t(memory2.get()), size},
            {std::uintptr_t(memory3.get()), size},
            {std::uintptr_t(memory4.get()), size},
            {std::uintptr_t(memory5.get()), size},
            {std::uintptr_t(memory6.get()), size},
            {std::uintptr_t(memory7.get()), size},
            {std::uintptr_t(memory8.get()), size},
            {0,                              0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_descRegionIdx == 0);
    }

    SECTION("Selected region is completly filled")
    {
        std::size_t pagesCount1 = 1;
        std::size_t pagesCount2 = 7;
        auto size1 = pageSize * pagesCount1;
        auto size2 = pageSize * pagesCount2;
        auto memory1 = test_alignedAlloc(pageSize, size1);
        auto memory2 = test_alignedAlloc(pageSize, size2);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory1.get()), size1},
            {std::uintptr_t(memory2.get()), size2},
            {0,                             0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_descRegionIdx == 0);
    }
}

TEST_CASE("Pages with page descriptors are properly reserved", "[page_allocator]")
{
    std::size_t pageSize = 256;
    PageAllocator pageAllocator;

    SECTION("Regions: 1(1)")
    {
        std::size_t pagesCount = 1;
        auto size = pageSize * pagesCount;
        auto memory = test_alignedAlloc(pageSize, size);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory.get()), size},
            {0,                            0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_descPagesCount == 1);

    }

    SECTION("Regions: 1(535), 2(87), 3(4)")
    {
        std::size_t pagesCount1 = 535;
        std::size_t pagesCount2 = 87;
        std::size_t pagesCount3 = 4;
        auto size1 = pageSize * pagesCount1;
        auto size2 = pageSize * pagesCount2;
        auto size3 = pageSize * pagesCount3;
        auto memory1 = test_alignedAlloc(pageSize, size1);
        auto memory2 = test_alignedAlloc(pageSize, size2);
        auto memory3 = test_alignedAlloc(pageSize, size3);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory1.get()), size1},
            {std::uintptr_t(memory2.get()), size2},
            {std::uintptr_t(memory3.get()), size3},
            {0,                             0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_descPagesCount == 79);
    }

    SECTION("All regions have 5 pages")
    {
        std::size_t pagesCount = 5;
        auto size = pageSize * pagesCount;
        auto memory1 = test_alignedAlloc(pageSize, size);
        auto memory2 = test_alignedAlloc(pageSize, size);
        auto memory3 = test_alignedAlloc(pageSize, size);
        auto memory4 = test_alignedAlloc(pageSize, size);
        auto memory5 = test_alignedAlloc(pageSize, size);
        auto memory6 = test_alignedAlloc(pageSize, size);
        auto memory7 = test_alignedAlloc(pageSize, size);
        auto memory8 = test_alignedAlloc(pageSize, size);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory1.get()), size},
            {std::uintptr_t(memory2.get()), size},
            {std::uintptr_t(memory3.get()), size},
            {std::uintptr_t(memory4.get()), size},
            {std::uintptr_t(memory5.get()), size},
            {std::uintptr_t(memory6.get()), size},
            {std::uintptr_t(memory7.get()), size},
            {std::uintptr_t(memory8.get()), size},
            {0,                              0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_descPagesCount == 5);
    }

    SECTION("Selected region is completly filled")
    {
        std::size_t pagesCount1 = 1;
        std::size_t pagesCount2 = 7;
        auto size1 = pageSize * pagesCount1;
        auto size2 = pageSize * pagesCount2;
        auto memory1 = test_alignedAlloc(pageSize, size1);
        auto memory2 = test_alignedAlloc(pageSize, size2);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory1.get()), size1},
            {std::uintptr_t(memory2.get()), size2},
            {0,                             0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_descPagesCount == 1);
    }
}

TEST_CASE("Group index is properly computed", "[page_allocator]")
{
    PageAllocator pageAllocator;
    std::map<std::size_t, std::pair<std::size_t, size_t>> idxRange = {
        {0, {0, 3}},
        {1, {4, 7}},
        {2, {8, 15}},
        {3, {16, 31}},
        {4, {32, 63}},
        {5, {64, 127}},
        {6, {128, 255}},
        {7, {256, 511}},
        {8, {512, 1023}},
        {9, {1024, 2047}},
        {10, {2048, 4095}},
        {11, {4096, 8191}},
        {12, {8192, 16383}},
        {13, {16384, 32767}},
        {14, {32768, 65535}},
        {15, {65536, 131071}},
        {16, {131072, 262143}},
        {17, {262144, 524287}},
        {18, {524288, 1048575}},
        {19, {1048576, 2097151}}
    };

    for (std::size_t i = 0; i < 0x200000; ++i) {
        auto idx = pageAllocator.groupIdx(i);
        REQUIRE(i >= idxRange[idx].first);
        REQUIRE(i <= idxRange[idx].second);
    }
}

TEST_CASE("Group is properly initialized", "[page_allocator]")
{
    PageAllocator pageAllocator;

    SECTION("Group has 1 page")
    {
        constexpr std::size_t groupSize = 1;
        std::array<std::byte, sizeof(Page) * groupSize> memory{};

        auto* group = reinterpret_cast<Page*>(std::begin(memory));
        pageAllocator.initGroup(group, groupSize);
        REQUIRE(group->groupSize() == groupSize);
    }

    SECTION("Group has 5 pages")
    {
        constexpr std::size_t groupSize = 5;
        std::array<std::byte, sizeof(Page) * groupSize> memory{};

        auto* group = reinterpret_cast<Page*>(std::begin(memory));
        pageAllocator.initGroup(group, groupSize);
        Page* firstPage = group;
        Page* lastPage = group + groupSize - 1;
        REQUIRE(firstPage->groupSize() == groupSize);
        REQUIRE(lastPage->groupSize() == groupSize);
    }
}

TEST_CASE("Group is properly cleared", "[page_allocator]")
{
    PageAllocator pageAllocator;

    SECTION("Group has 1 page")
    {
        constexpr std::size_t groupSize = 1;
        std::array<std::byte, sizeof(Page) * groupSize> memory{};

        auto* group = reinterpret_cast<Page*>(std::begin(memory));
        pageAllocator.initGroup(group, groupSize);
        pageAllocator.clearGroup(group);
        REQUIRE(group->groupSize() == 0);
    }

    SECTION("Group has 5 pages")
    {
        constexpr std::size_t groupSize = 5;
        std::array<std::byte, sizeof(Page) * groupSize> memory{};

        auto* group = reinterpret_cast<Page*>(std::begin(memory));
        pageAllocator.initGroup(group, groupSize);
        pageAllocator.clearGroup(group);
        Page* firstPage = group;
        Page* lastPage = group + groupSize - 1;
        REQUIRE(firstPage->groupSize() == 0);
        REQUIRE(lastPage->groupSize() == 0);
    }
}

TEST_CASE("Group is properly added to list", "[page_allocator]")
{
    PageAllocator pageAllocator;
    std::size_t groupCount = 0;

    SECTION("1 group is stored at index 0")
    {
        constexpr std::size_t groupSize = 3;
        std::array<std::byte, sizeof(Page) * groupSize> memory{};

        auto* group = reinterpret_cast<Page*>(std::begin(memory));
        pageAllocator.initGroup(group, groupSize);
        pageAllocator.addGroup(group);

        for (Page* it = pageAllocator.m_freeGroupLists[0]; it != nullptr; it = it->nextGroup()) {
            REQUIRE(it->groupSize() == groupSize);
            REQUIRE(!it->isUsed());
            ++groupCount;
        }

        REQUIRE(groupCount == 1);
        REQUIRE(pageAllocator.m_freePagesCount == groupSize);
    }

    SECTION("3 groups are stored at index 0")
    {
        constexpr std::size_t groupSize = 3;
        std::array<std::byte, sizeof(Page) * groupSize> memory1{};
        std::array<std::byte, sizeof(Page) * groupSize> memory2{};
        std::array<std::byte, sizeof(Page) * groupSize> memory3{};

        auto* group1 = reinterpret_cast<Page*>(std::begin(memory1));
        auto* group2 = reinterpret_cast<Page*>(std::begin(memory2));
        auto* group3 = reinterpret_cast<Page*>(std::begin(memory3));
        pageAllocator.initGroup(group1, groupSize);
        pageAllocator.initGroup(group2, groupSize);
        pageAllocator.initGroup(group3, groupSize);
        pageAllocator.addGroup(group1);
        pageAllocator.addGroup(group2);
        pageAllocator.addGroup(group3);

        for (Page* group = pageAllocator.m_freeGroupLists[0]; group != nullptr; group = group->nextGroup()) {
            REQUIRE(group->groupSize() == groupSize);
            REQUIRE(!group->isUsed());
            ++groupCount;
        }

        REQUIRE(groupCount == 3);
        REQUIRE(pageAllocator.m_freePagesCount == 3 * groupSize);
    }

    SECTION("1 group is stored at index 4")
    {
        constexpr std::size_t groupSize = 34;
        std::array<std::byte, sizeof(Page) * groupSize> memory{};

        auto* group = reinterpret_cast<Page*>(std::begin(memory));
        pageAllocator.initGroup(group, groupSize);
        pageAllocator.addGroup(group);

        for (Page* it = pageAllocator.m_freeGroupLists[4]; it != nullptr; it = it->nextGroup()) {
            REQUIRE(it->groupSize() == groupSize);
            REQUIRE(!it->isUsed());
            ++groupCount;
        }

        REQUIRE(groupCount == 1);
        REQUIRE(pageAllocator.m_freePagesCount == groupSize);
    }

    SECTION("3 groups are stored at index 4")
    {
        constexpr std::size_t groupSize = 34;
        std::array<std::byte, sizeof(Page) * groupSize> memory1{};
        std::array<std::byte, sizeof(Page) * groupSize> memory2{};
        std::array<std::byte, sizeof(Page) * groupSize> memory3{};

        auto* group1 = reinterpret_cast<Page*>(std::begin(memory1));
        auto* group2 = reinterpret_cast<Page*>(std::begin(memory2));
        auto* group3 = reinterpret_cast<Page*>(std::begin(memory3));
        pageAllocator.initGroup(group1, groupSize);
        pageAllocator.initGroup(group2, groupSize);
        pageAllocator.initGroup(group3, groupSize);
        pageAllocator.addGroup(group1);
        pageAllocator.addGroup(group2);
        pageAllocator.addGroup(group3);

        for (Page* group = pageAllocator.m_freeGroupLists[4]; group != nullptr; group = group->nextGroup()) {
            REQUIRE(group->groupSize() == groupSize);
            REQUIRE(!group->isUsed());
            ++groupCount;
        }

        REQUIRE(groupCount == 3);
        REQUIRE(pageAllocator.m_freePagesCount == 3 * groupSize);
    }
}

TEST_CASE("Group is properly removed from list at index 0", "[page_allocator]")
{
    PageAllocator pageAllocator;
    constexpr std::size_t groupSize = 3;
    std::size_t pagesCount = 0;
    std::array<std::byte, sizeof(Page) * groupSize> memory1{};
    std::array<std::byte, sizeof(Page) * groupSize> memory2{};
    std::array<std::byte, sizeof(Page) * groupSize> memory3{};

    auto* group1 = reinterpret_cast<Page*>(std::begin(memory1));
    auto* group2 = reinterpret_cast<Page*>(std::begin(memory2));
    auto* group3 = reinterpret_cast<Page*>(std::begin(memory3));
    pageAllocator.initGroup(group1, groupSize);
    pageAllocator.initGroup(group2, groupSize);
    pageAllocator.initGroup(group3, groupSize);
    pageAllocator.addGroup(group1);
    pageAllocator.addGroup(group2);
    pageAllocator.addGroup(group3);

    std::size_t freeCount = pageAllocator.m_freePagesCount;

    SECTION("First of three group is removed")
    {
        pageAllocator.removeGroup(group1);
        int idx = 0;
        for (Page* group = pageAllocator.m_freeGroupLists[0]; group != nullptr; group = group->nextGroup()) {
            REQUIRE(group->groupSize() == groupSize);
            REQUIRE(group == (idx == 0 ? group3 : group2));

            ++pagesCount;
            ++idx;
        }

        for (std::size_t i = 0; i < group1->groupSize(); ++i) {
            auto* page = group1 + i;
            REQUIRE(page->isUsed());
        }

        REQUIRE(pageAllocator.m_freePagesCount == freeCount - group1->groupSize());
    }

    SECTION("Second of three group is removed")
    {
        pageAllocator.removeGroup(group2);
        int idx = 0;
        for (Page* group = pageAllocator.m_freeGroupLists[0]; group != nullptr; group = group->nextGroup()) {
            REQUIRE(group->groupSize() == groupSize);
            REQUIRE(group == (idx == 0 ? group3 : group1));

            ++pagesCount;
            ++idx;
        }

        for (std::size_t i = 0; i < group2->groupSize(); ++i) {
            auto* page = group2 + i;
            REQUIRE(page->isUsed());
        }

        REQUIRE(pageAllocator.m_freePagesCount == freeCount - group2->groupSize());
    }

    SECTION("Third of three group is removed")
    {
        pageAllocator.removeGroup(group3);
        int idx = 0;
        for (Page* group = pageAllocator.m_freeGroupLists[0]; group != nullptr; group = group->nextGroup()) {
            REQUIRE(group->groupSize() == groupSize);
            REQUIRE(group == (idx == 0 ? group2 : group1));

            ++pagesCount;
            ++idx;
        }

        for (std::size_t i = 0; i < group3->groupSize(); ++i) {
            auto* page = group3 + i;
            REQUIRE(page->isUsed());
        }

        REQUIRE(pageAllocator.m_freePagesCount == freeCount - group3->groupSize());
    }

    REQUIRE(pagesCount == 2);
}

TEST_CASE("Group is properly removed from list at index 4", "[page_allocator]")
{
    PageAllocator pageAllocator;
    constexpr std::size_t groupSize = 34;
    std::size_t pagesCount = 0;
    std::array<std::byte, sizeof(Page) * groupSize> memory1{};
    std::array<std::byte, sizeof(Page) * groupSize> memory2{};
    std::array<std::byte, sizeof(Page) * groupSize> memory3{};

    auto* group1 = reinterpret_cast<Page*>(std::begin(memory1));
    auto* group2 = reinterpret_cast<Page*>(std::begin(memory2));
    auto* group3 = reinterpret_cast<Page*>(std::begin(memory3));
    pageAllocator.initGroup(group1, groupSize);
    pageAllocator.initGroup(group2, groupSize);
    pageAllocator.initGroup(group3, groupSize);
    pageAllocator.addGroup(group1);
    pageAllocator.addGroup(group2);
    pageAllocator.addGroup(group3);

    std::size_t freeCount = pageAllocator.m_freePagesCount;

    SECTION("First of three group is removed")
    {
        pageAllocator.removeGroup(group1);
        int idx = 0;
        for (Page* group = pageAllocator.m_freeGroupLists[4]; group != nullptr; group = group->nextGroup()) {
            REQUIRE(group->groupSize() == groupSize);
            REQUIRE(group == (idx == 0 ? group3 : group2));

            ++pagesCount;
            ++idx;
        }

        for (std::size_t i = 0; i < group1->groupSize(); ++i) {
            auto* page = group1 + i;
            REQUIRE(page->isUsed());
        }

        REQUIRE(pageAllocator.m_freePagesCount == freeCount - group1->groupSize());
    }

    SECTION("Second of three group is removed")
    {
        pageAllocator.removeGroup(group2);
        int idx = 0;
        for (Page* group = pageAllocator.m_freeGroupLists[4]; group != nullptr; group = group->nextGroup()) {
            REQUIRE(group->groupSize() == groupSize);
            REQUIRE(group == (idx == 0 ? group3 : group1));

            ++pagesCount;
            ++idx;
        }

        for (std::size_t i = 0; i < group2->groupSize(); ++i) {
            auto* page = group2 + i;
            REQUIRE(page->isUsed());
        }

        REQUIRE(pageAllocator.m_freePagesCount == freeCount - group1->groupSize());
    }

    SECTION("Third of three group is removed")
    {
        pageAllocator.removeGroup(group3);
        int idx = 0;
        for (Page* group = pageAllocator.m_freeGroupLists[4]; group != nullptr; group = group->nextGroup()) {
            REQUIRE(group->groupSize() == groupSize);
            REQUIRE(group == (idx == 0 ? group2 : group1));

            ++pagesCount;
            ++idx;
        }

        for (std::size_t i = 0; i < group3->groupSize(); ++i) {
            auto* page = group3 + i;
            REQUIRE(page->isUsed());
        }

        REQUIRE(pageAllocator.m_freePagesCount == freeCount - group1->groupSize());
    }

    REQUIRE(pagesCount == 2);
}

TEST_CASE("Group is properly splitted", "[page_allocator]")
{
    constexpr std::size_t groupSize = 10;
    std::array<std::byte, sizeof(Page) * groupSize> memory{};

    auto* group = reinterpret_cast<Page*>(std::begin(memory));

    PageAllocator pageAllocator;
    pageAllocator.initGroup(group, groupSize);

    Page* firstGroup = nullptr;
    Page* secondGroup = nullptr;
    std::size_t splitSize = 0;

    SECTION("Split size is equal to group size")
    {
        splitSize = groupSize;
        std::tie(firstGroup, secondGroup) = pageAllocator.splitGroup(group, splitSize);
        REQUIRE(firstGroup);
        REQUIRE(firstGroup->groupSize() == splitSize);
        REQUIRE(secondGroup == nullptr);
    }

    SECTION("First group should have 1 page")
    {
        splitSize = 1;
        std::tie(firstGroup, secondGroup) = pageAllocator.splitGroup(group, splitSize);
        REQUIRE(firstGroup);
        REQUIRE(secondGroup);
        REQUIRE(firstGroup->groupSize() == splitSize);
        REQUIRE(secondGroup->groupSize() == groupSize - splitSize);
    }

    SECTION("First group should have 3 pages")
    {
        splitSize = 3;
        std::tie(firstGroup, secondGroup) = pageAllocator.splitGroup(group, splitSize);
        REQUIRE(firstGroup);
        REQUIRE(secondGroup);
        REQUIRE(firstGroup->groupSize() == splitSize);
        REQUIRE(secondGroup->groupSize() == groupSize - splitSize);
    }

    SECTION("First group should have 5 pages")
    {
        splitSize = 5;
        std::tie(firstGroup, secondGroup) = pageAllocator.splitGroup(group, splitSize);
        REQUIRE(firstGroup);
        REQUIRE(secondGroup);
        REQUIRE(firstGroup->groupSize() == splitSize);
        REQUIRE(secondGroup->groupSize() == groupSize - splitSize);
    }
}

TEST_CASE("Group is properly joined", "[page_allocator]")
{
    constexpr std::size_t groupSize = 10;
    std::array<std::byte, sizeof(Page) * groupSize> memory{};

    auto* group = reinterpret_cast<Page*>(std::begin(memory));

    PageAllocator pageAllocator;
    pageAllocator.initGroup(group, groupSize);

    std::size_t splitSize = 0;

    SECTION("First group should have 1 page")
    {
        splitSize = 1;
    }

    SECTION("First group should have 3 pages")
    {
        splitSize = 3;
    }

    SECTION("First group should have 5 pages")
    {
        splitSize = 5;
    }

    Page* firstGroup = nullptr;
    Page* secondGroup = nullptr;
    std::tie(firstGroup, secondGroup) = pageAllocator.splitGroup(group, splitSize);
    Page* joinedGroup = pageAllocator.joinGroup(firstGroup, secondGroup);
    REQUIRE(joinedGroup);
    REQUIRE(joinedGroup->groupSize() == groupSize);
}

TEST_CASE("Page is properly verified as valid", "[page_allocator]")
{
    std::size_t pageSize = 256;
    PageAllocator pageAllocator;

    std::size_t pagesCount1 = 535;
    std::size_t pagesCount2 = 87;
    std::size_t pagesCount3 = 4;
    auto size1 = pageSize * pagesCount1;
    auto size2 = pageSize * pagesCount2;
    auto size3 = pageSize * pagesCount3;
    auto memory1 = test_alignedAlloc(pageSize, size1);
    auto memory2 = test_alignedAlloc(pageSize, size2);
    auto memory3 = test_alignedAlloc(pageSize, size3);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory1.get()), size1},
        {std::uintptr_t(memory2.get()), size2},
        {std::uintptr_t(memory3.get()), size3},
        {0,                             0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    SECTION("Page points to the first page desc")
    {
        auto* page = pageAllocator.m_pagesHead;
        REQUIRE(pageAllocator.isValidPage(page));
    }

    SECTION("Page points to the last page desc")
    {
        auto* page = pageAllocator.m_pagesTail;
        REQUIRE(pageAllocator.isValidPage(page));
    }

    SECTION("Page points in the middle of page desc list")
    {
        auto* page = pageAllocator.m_pagesHead + pageAllocator.m_descPagesCount / 2;
        REQUIRE(pageAllocator.isValidPage(page));
    }

    SECTION("Page points before first page desc")
    {
        auto* page = pageAllocator.m_pagesHead - 1;
        REQUIRE(!pageAllocator.isValidPage(page));
    }

    SECTION("Page points after last page desc")
    {
        auto* page = pageAllocator.m_pagesTail + 1;
        REQUIRE(!pageAllocator.isValidPage(page));
    }
}

TEST_CASE("Region is properly resolved from address", "[page_allocator]")
{
    std::size_t pageSize = 256;
    PageAllocator pageAllocator;

    std::size_t pagesCount1 = 535;
    std::size_t pagesCount2 = 87;
    std::size_t pagesCount3 = 4;
    auto size1 = pageSize * pagesCount1;
    auto size2 = pageSize * pagesCount2;
    auto size3 = pageSize * pagesCount3;
    auto memory1 = test_alignedAlloc(pageSize, size1);
    auto memory2 = test_alignedAlloc(pageSize, size2);
    auto memory3 = test_alignedAlloc(pageSize, size3);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory1.get()), size1},
        {std::uintptr_t(memory2.get()), size2},
        {std::uintptr_t(memory3.get()), size3},
        {0,                             0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    SECTION("Address points to the beginning of the first region")
    {
        auto* region = pageAllocator.getRegion(std::uintptr_t(memory1.get()));
        REQUIRE(region == &pageAllocator.m_regionsInfo[0]);
    }

    SECTION("Address points to the end of the first region")
    {
        auto* region = pageAllocator.getRegion(std::uintptr_t(memory1.get()) + size1 - 1);
        REQUIRE(region == &pageAllocator.m_regionsInfo[0]);
    }

    SECTION("Address points to the middle of the first region")
    {
        auto* region = pageAllocator.getRegion(std::uintptr_t(memory1.get()) + size1 / 2);
        REQUIRE(region == &pageAllocator.m_regionsInfo[0]);
    }

    SECTION("Address points to the beginning of the third region")
    {
        auto* region = pageAllocator.getRegion(std::uintptr_t(memory3.get()));
        REQUIRE(region == &pageAllocator.m_regionsInfo[2]);
    }

    SECTION("Address points to the end of the third region")
    {
        auto* region = pageAllocator.getRegion(std::uintptr_t(memory3.get()) + size3 - 1);
        REQUIRE(region == &pageAllocator.m_regionsInfo[2]);
    }

    SECTION("Address points to the middle of the third region")
    {
        auto* region = pageAllocator.getRegion(std::uintptr_t(memory3.get()) + size3 / 2);
        REQUIRE(region == &pageAllocator.m_regionsInfo[2]);
    }

    SECTION("Address points to outside of any region")
    {
        auto* region = pageAllocator.getRegion(std::uintptr_t(memory1.get()) - 1);
        REQUIRE(region == nullptr);
    }
}

TEST_CASE("Pages are correctly resolved from address", "[page_allocator]")
{
    std::size_t pageSize = 256;
    PageAllocator pageAllocator;

    std::size_t pagesCount1 = 535;
    std::size_t pagesCount2 = 87;
    std::size_t pagesCount3 = 4;
    auto size1 = pageSize * pagesCount1;
    auto size2 = pageSize * pagesCount2;
    auto size3 = pageSize * pagesCount3;
    auto memory1 = test_alignedAlloc(pageSize, size1);
    auto memory2 = test_alignedAlloc(pageSize, size2);
    auto memory3 = test_alignedAlloc(pageSize, size3);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory1.get()), size1},
        {std::uintptr_t(memory2.get()), size2},
        {std::uintptr_t(memory3.get()), size3},
        {0,                             0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    Page* page = nullptr;

    SECTION("Address it outside any region")
    {
        page = pageAllocator.getPage(std::uintptr_t(memory1.get()) - 1);
        REQUIRE(page == nullptr);
    }

    SECTION("Address points to the beginning of the first region")
    {
        page = pageAllocator.getPage(std::uintptr_t(memory1.get()));
        REQUIRE(page);
        REQUIRE(page->address() == std::uintptr_t(memory1.get()));
    }

    SECTION("Address points to the beginning of the second region")
    {
        page = pageAllocator.getPage(std::uintptr_t(memory2.get()));
        REQUIRE(page);
        REQUIRE(page->address() == std::uintptr_t(memory2.get()));
    }

    SECTION("Address points to the end of the first region")
    {
        page = pageAllocator.getPage(std::uintptr_t(memory1.get()) + size1 - 1);
        REQUIRE(page);
        REQUIRE(page->address() == (std::uintptr_t(memory1.get()) + (pagesCount1 - 1) * pageSize));
    }

    SECTION("Address points to the end of the second region")
    {
        page = pageAllocator.getPage(std::uintptr_t(memory2.get()) + size2 - 1);
        REQUIRE(page);
        REQUIRE(page->address() == (std::uintptr_t(memory2.get()) + (pagesCount2 - 1) * pageSize));
    }

    SECTION("Address points to the 16th page in the first region")
    {
        page = pageAllocator.getPage(std::uintptr_t(memory1.get()) + 16 * pageSize);
        REQUIRE(page);
        REQUIRE(page->address() == (std::uintptr_t(memory1.get()) + 16 * pageSize));
    }

    SECTION("Address points to the 7th page in the second region")
    {
        page = pageAllocator.getPage(std::uintptr_t(memory2.get()) + 7 * pageSize);
        REQUIRE(page);
        REQUIRE(page->address() == (std::uintptr_t(memory2.get()) + 7 * pageSize));
    }

    SECTION("Address points to in the middle of the second page in the first region")
    {
        page = pageAllocator.getPage(std::uintptr_t(memory1.get()) + pageSize + pageSize / 2);
        REQUIRE(page);
        REQUIRE(page->address() == (std::uintptr_t(memory1.get()) + pageSize));
    }

    SECTION("Address points to in the middle of the third page in the third region")
    {
        page = pageAllocator.getPage(std::uintptr_t(memory3.get()) + 2 * pageSize + pageSize / 2);
        REQUIRE(page);
        REQUIRE(page->address() == (std::uintptr_t(memory3.get()) + 2 * pageSize));
    }
}

TEST_CASE("Stats are properly initialized", "[page_allocator]")
{
    std::size_t pageSize = 256;
    PageAllocator pageAllocator;

    std::size_t pagesCount1 = 535;
    std::size_t pagesCount2 = 87;
    std::size_t pagesCount3 = 4;
    auto size1 = pageSize * pagesCount1;
    auto size2 = pageSize * pagesCount2;
    auto size3 = pageSize * pagesCount3;
    auto memory1 = test_alignedAlloc(pageSize, size1);
    auto memory2 = test_alignedAlloc(pageSize, size2);
    auto memory3 = test_alignedAlloc(pageSize, size3);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory1.get()), size1},
        {std::uintptr_t(memory2.get()), size2},
        {std::uintptr_t(memory3.get()), size3},
        {0,                             0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    auto stats = pageAllocator.getStats();
    REQUIRE(stats.pageSize == pageAllocator.m_pageSize);
    REQUIRE(stats.pagesCount == pageAllocator.m_pagesCount);
    REQUIRE(stats.freePagesCount == pageAllocator.m_freePagesCount);
    REQUIRE(stats.descRegionIdx == pageAllocator.m_descRegionIdx);
    REQUIRE(stats.descPagesCount == pageAllocator.m_descPagesCount);
}

TEST_CASE("Pages are correctly allocated", "[page_allocator]")
{
    std::size_t pageSize = 256;
    PageAllocator pageAllocator;

    std::size_t pagesCount1 = 535;
    std::size_t pagesCount2 = 87;
    std::size_t pagesCount3 = 4;
    auto size1 = pageSize * pagesCount1;
    auto size2 = pageSize * pagesCount2;
    auto size3 = pageSize * pagesCount3;
    auto memory1 = test_alignedAlloc(pageSize, size1);
    auto memory2 = test_alignedAlloc(pageSize, size2);
    auto memory3 = test_alignedAlloc(pageSize, size3);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory1.get()), size1},
        {std::uintptr_t(memory2.get()), size2},
        {std::uintptr_t(memory3.get()), size3},
        {0,                             0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));
    auto freePages = pageAllocator.m_freePagesCount;
    std::vector<Page*> pages;

    SECTION("Allocating 0 pages")
    {
        pages.push_back(pageAllocator.allocate(0));
        REQUIRE(pages.back() == nullptr);
    }

    SECTION("Allocating more pages than free count")
    {
        pages.push_back(pageAllocator.allocate(pagesCount1 + pagesCount2 + pagesCount3 + 1));
        REQUIRE(pages.back() == nullptr);
    }

    SECTION("Allocating more pages than biggest free continues group")
    {
        pages.push_back(pageAllocator.allocate(pagesCount1 + 1));
        REQUIRE(pages.back() == nullptr);
    }

    SECTION("Allocating 1 page")
    {
        pages.push_back(pageAllocator.allocate(1));
        REQUIRE(pages.back());
        REQUIRE(pages.back()->address() == std::uintptr_t(memory3.get()));
        REQUIRE(pageAllocator.m_freePagesCount == freePages - 1);

        std::size_t idx1Count = 0;
        for (Page* group = pageAllocator.m_freeGroupLists[1]; group != nullptr; group = group->nextGroup())
            ++idx1Count;

        REQUIRE(idx1Count == 0);

        std::size_t idx0Count = 0;
        for (Page* group = pageAllocator.m_freeGroupLists[0]; group != nullptr; group = group->nextGroup())
            ++idx0Count;

        REQUIRE(idx0Count == 1);
        REQUIRE(pageAllocator.m_freeGroupLists[0]->groupSize() == pagesCount3 - 1);
    }

    SECTION("Allocating 17 pages")
    {
        pages.push_back(pageAllocator.allocate(17));
        REQUIRE(pages.back());
        REQUIRE(pages.back()->address() == std::uintptr_t(memory1.get()));
        REQUIRE(pageAllocator.m_freePagesCount == freePages - 17);

        std::size_t idx8Count = 0;
        for (Page* group = pageAllocator.m_freeGroupLists[8]; group != nullptr; group = group->nextGroup())
            ++idx8Count;

        REQUIRE(idx8Count == 1);
        REQUIRE(pageAllocator.m_freeGroupLists[8]->groupSize() == pagesCount1 - 17);
    }

    SECTION("Allocating whole region")
    {
        pages.push_back(pageAllocator.allocate(pagesCount1));
        REQUIRE(pages.back());
        REQUIRE(pages.back()->address() == std::uintptr_t(memory1.get()));
        REQUIRE(pageAllocator.m_freePagesCount == freePages - pagesCount1);
    }


    SECTION("Allocate 1 page 4 times")
    {
        for (int i = 0; i < 4; ++i) {
            pages.push_back(pageAllocator.allocate(1));
            REQUIRE(pages[i]);
            REQUIRE(pages[i]->address() == std::uintptr_t(memory3.get()) + i * pageSize);
            REQUIRE(pageAllocator.m_freePagesCount == freePages - i - 1);
        }
    }

    SECTION("Only 2 pages are left in each region")
    {
        std::size_t allocated = 0;

        pages.push_back(pageAllocator.allocate(pagesCount3 - 2));
        allocated += pagesCount3 - 2;
        REQUIRE(pages.back());
        REQUIRE(pages.back()->address() == std::uintptr_t(memory3.get()));
        REQUIRE(pageAllocator.m_freePagesCount == freePages - allocated);

        pages.push_back(pageAllocator.allocate(pagesCount2 - pageAllocator.m_descPagesCount - 2));
        allocated += pagesCount2 - pageAllocator.m_descPagesCount - 2;
        REQUIRE(pages.back());
        REQUIRE(pages.back()->address() == std::uintptr_t(memory2.get() + pageAllocator.m_descPagesCount * pageSize));
        REQUIRE(pageAllocator.m_freePagesCount == freePages - 8);

        pages.push_back(pageAllocator.allocate(pagesCount1 - 2));
        allocated += pagesCount1 - 2;
        REQUIRE(pages.back());
        REQUIRE(pages.back()->address() == std::uintptr_t(memory1.get()));
        REQUIRE(pageAllocator.m_freePagesCount == freePages - allocated);
    }

    SECTION("Allocate all pages one by one")
    {
        for (int i = 0; i < freePages; ++i) {
            pages.push_back(pageAllocator.allocate(1));
            REQUIRE(pages.back());
            REQUIRE(pageAllocator.m_freePagesCount == freePages - i - 1);
            REQUIRE(pageAllocator.getPage(pages.back()->address()) == pages[i]);
        }

        for (int i = 0; i < PageAllocator::MAX_GROUP_IDX; ++i) {
            std::size_t idxCount = 0;
            for (Page* group = pageAllocator.m_freeGroupLists[i]; group != nullptr; group = group->nextGroup())
                ++idxCount;

            REQUIRE(idxCount == 0);
        }
    }
}

TEST_CASE("Pages are correctly released", "[page_allocator]")
{
    std::size_t pageSize = 256;
    PageAllocator pageAllocator;

    std::size_t pagesCount1 = 535;
    std::size_t pagesCount2 = 87;
    std::size_t pagesCount3 = 4;
    auto size1 = pageSize * pagesCount1;
    auto size2 = pageSize * pagesCount2;
    auto size3 = pageSize * pagesCount3;
    auto memory1 = test_alignedAlloc(pageSize, size1);
    auto memory2 = test_alignedAlloc(pageSize, size2);
    auto memory3 = test_alignedAlloc(pageSize, size3);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory1.get()), size1},
        {std::uintptr_t(memory2.get()), size2},
        {std::uintptr_t(memory3.get()), size3},
        {0,                             0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));
    auto freePages = pageAllocator.m_freePagesCount;
    std::vector<Page*> pages;

    SECTION("Releasing nullptr is a valid operation")
    {
        pageAllocator.release(nullptr);
    }

    SECTION("Releasing 1 page")
    {
        pages.push_back(pageAllocator.allocate(1));
        pageAllocator.release(pages.back());
    }

    SECTION("Releasing 17 pages")
    {
        pages.push_back(pageAllocator.allocate(17));
        pageAllocator.release(pages.back());
    }

    SECTION("Releasing whole region")
    {
        pages.push_back(pageAllocator.allocate(pagesCount1));
        pageAllocator.release(pages.back());
    }

    SECTION("Allocate 1 page 4 times, release from first")
    {
        for (std::size_t i = 0; i < 4; ++i)
            pages.push_back(pageAllocator.allocate(1));

        for (auto* page : pages)
            pageAllocator.release(page);
    }

    SECTION("Allocate 1 page 4 times, release from last")
    {
        for (std::size_t i = 0; i < 4; ++i)
            pages.push_back(pageAllocator.allocate(1));

        for (std::size_t i = 0; i < pages.size(); ++i)
            pageAllocator.release(pages[pages.size() - 1 - i]);
    }

    SECTION("Only 2 pages are left in each region, release from first")
    {
        pages.push_back(pageAllocator.allocate(pagesCount3 - 2));
        pages.push_back(pageAllocator.allocate(pagesCount2 - pageAllocator.m_descPagesCount - 2));
        pages.push_back(pageAllocator.allocate(pagesCount1 - 2));

        for (auto* page : pages)
            pageAllocator.release(page);
    }

    SECTION("Only 2 pages are left in each region, release from last")
    {
        pages.push_back(pageAllocator.allocate(pagesCount3 - 2));
        pages.push_back(pageAllocator.allocate(pagesCount2 - pageAllocator.m_descPagesCount - 2));
        pages.push_back(pageAllocator.allocate(pagesCount1 - 2));

        for (std::size_t i = 0; i < pages.size(); ++i)
            pageAllocator.release(pages[pages.size() - 1 - i]);
    }

    SECTION("Allocate all pages one by one, release from first")
    {
        for (int i = 0; i < freePages; ++i)
            pages.push_back(pageAllocator.allocate(1));

        for (auto* page : pages)
            pageAllocator.release(page);
    }

    SECTION("Allocate all pages one by one, release from first")
    {
        for (int i = 0; i < freePages; ++i)
            pages.push_back(pageAllocator.allocate(1));

        for (std::size_t i = 0; i < pages.size(); ++i)
            pageAllocator.release(pages[pages.size() - 1 - i]);
    }

    REQUIRE(pageAllocator.m_freePagesCount == freePages);

    std::size_t idx1Count = 0;
    for (Page* group = pageAllocator.m_freeGroupLists[1]; group != nullptr; group = group->nextGroup())
        ++idx1Count;

    REQUIRE(idx1Count == 1);
    REQUIRE(pageAllocator.m_freeGroupLists[1]->groupSize() == pagesCount3);

    std::size_t idx2Count = 0;
    for (Page* group = pageAllocator.m_freeGroupLists[2]; group != nullptr; group = group->nextGroup())
        ++idx2Count;

    REQUIRE(idx2Count == 1);
    REQUIRE(pageAllocator.m_freeGroupLists[2]->groupSize() == pagesCount2 - pageAllocator.m_descPagesCount);

    std::size_t idx8Count = 0;
    for (Page* group = pageAllocator.m_freeGroupLists[8]; group != nullptr; group = group->nextGroup())
        ++idx8Count;

    REQUIRE(idx8Count == 1);
    REQUIRE(pageAllocator.m_freeGroupLists[8]->groupSize() == pagesCount1);
}

TEST_CASE("Integration tests (long-term)", "[page_allocator][integration][.]")
{
    using namespace std::chrono_literals;
    constexpr auto testDuration = 30min;
    constexpr int allocationsCount = 100;

    std::size_t pageSize = 256;
    PageAllocator pageAllocator;

    std::size_t pagesCount1 = 535;
    std::size_t pagesCount2 = 87;
    std::size_t pagesCount3 = 4;
    auto size1 = pageSize * pagesCount1;
    auto size2 = pageSize * pagesCount2;
    auto size3 = pageSize * pagesCount3;
    auto memory1 = test_alignedAlloc(pageSize, size1);
    auto memory2 = test_alignedAlloc(pageSize, size2);
    auto memory3 = test_alignedAlloc(pageSize, size3);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory1.get()), size1},
        {std::uintptr_t(memory2.get()), size2},
        {std::uintptr_t(memory3.get()), size3},
        {0,                             0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));
    auto freePages = pageAllocator.m_freePagesCount;

    // Initialize random number generator.
    std::random_device randomDevice;
    std::mt19937 randomGenerator(randomDevice());
    std::uniform_int_distribution<std::size_t> distribution(0, freePages / 4);

    std::array<Page*, allocationsCount> pages{};

    for (auto start = test_currentTime(); !test_timeElapsed(start, testDuration);) {
        pages.fill(nullptr);

        // Allocate pages.
        for (auto*& page : pages) {
            auto n = distribution(randomGenerator);
            page = pageAllocator.allocate(n);
        }

        // Release pages.
        for (auto* page : pages)
            pageAllocator.release(page);

        REQUIRE(pageAllocator.m_freePagesCount == freePages);

        std::size_t idx1Count = 0;
        for (Page* group = pageAllocator.m_freeGroupLists[1]; group != nullptr; group = group->nextGroup())
            ++idx1Count;

        REQUIRE(idx1Count == 1);
        REQUIRE(pageAllocator.m_freeGroupLists[1]->groupSize() == pagesCount3);

        std::size_t idx2Count = 0;
        for (Page* group = pageAllocator.m_freeGroupLists[2]; group != nullptr; group = group->nextGroup())
            ++idx2Count;

        REQUIRE(idx2Count == 1);
        REQUIRE(pageAllocator.m_freeGroupLists[2]->groupSize() == pagesCount2 - pageAllocator.m_descPagesCount);

        std::size_t idx8Count = 0;
        for (Page* group = pageAllocator.m_freeGroupLists[8]; group != nullptr; group = group->nextGroup())
            ++idx8Count;

        REQUIRE(idx8Count == 1);
        REQUIRE(pageAllocator.m_freeGroupLists[8]->groupSize() == pagesCount1);
    }
}
