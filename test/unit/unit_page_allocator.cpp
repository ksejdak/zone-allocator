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
#include <group.hpp>
#include <page.hpp>
#include <page_allocator.hpp>
#include <test_utils.hpp>

#include <catch2/catch.hpp>

#include <array>
#include <cstdlib>
#include <cstring>
#include <map>
#include <vector>

namespace memory {

TEST_CASE("Page allocator is properly cleared", "[unit][page_allocator]")
{
    PageAllocator pageAllocator;
    constexpr int cPattern = 0x5a;
    std::memset(reinterpret_cast<void*>(&pageAllocator), cPattern, sizeof(PageAllocator));

    pageAllocator.clear();

    auto stats = pageAllocator.getStats();
    REQUIRE(stats.totalMemorySize == 0);
    REQUIRE(stats.effectiveMemorySize == 0);
    REQUIRE(stats.userMemorySize == 0);
    REQUIRE(stats.freeMemorySize == 0);
    REQUIRE(stats.pageSize == 0);
    REQUIRE(stats.totalPagesCount == 0);
    REQUIRE(stats.reservedPagesCount == 0);
    REQUIRE(stats.freePagesCount == 0);
}

TEST_CASE("Page size is correctly validated", "[unit][page_allocator]")
{
    PageAllocator pageAllocator;
    std::size_t pageSize = 0;
    bool isValidPageSize = false;

    SECTION("Page size is smaller than the minimal value")
    {
        pageSize = PageAllocator::cMinPageSize - 3;
        isValidPageSize = detail::isValidPageSize(pageSize);
        REQUIRE(!isValidPageSize);
    }

    SECTION("Page size is equal to the minimal value")
    {
        pageSize = PageAllocator::cMinPageSize;
        isValidPageSize = detail::isValidPageSize(pageSize);
        REQUIRE(isValidPageSize);
    }

    SECTION("Page size is bigger than the minimal value, but not the power of 2")
    {
        pageSize = (2 * PageAllocator::cMinPageSize) + 1;
        isValidPageSize = detail::isValidPageSize(pageSize);
        REQUIRE(!isValidPageSize);
    }

    SECTION("Page size is bigger than the minimal value and a power of 2")
    {
        pageSize = 256; // NOLINT
        isValidPageSize = detail::isValidPageSize(pageSize);
        REQUIRE(isValidPageSize);
    }

    std::size_t pagesCount = 1;
    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);

    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {{{std::uintptr_t(memory.get()), size}, {0, 0}}};

    REQUIRE(pageAllocator.init(regions.data(), pageSize) == isValidPageSize);
}

TEST_CASE("Pages are correctly counted", "[unit][page_allocator]")
{
    constexpr std::size_t cPageSize = 256;
    PageAllocator pageAllocator;

    SECTION("Regions: 1(1)")
    {
        constexpr std::size_t cPagesCount = 1;
        auto size = cPageSize * cPagesCount;
        auto memory = test::alignedAlloc(cPageSize, size);

        constexpr int cRegionsCount = 2;
        std::array<Region, cRegionsCount> regions = {{{std::uintptr_t(memory.get()), size}, {0, 0}}};

        REQUIRE(pageAllocator.init(regions.data(), cPageSize));
        auto stats = pageAllocator.getStats();
        REQUIRE(stats.totalPagesCount == cPagesCount);
    }

    SECTION("Regions: 1(535), 2(87), 3(4)")
    {
        constexpr std::size_t cPagesCount1 = 535;
        constexpr std::size_t cPagesCount2 = 87;
        constexpr std::size_t cPagesCount3 = 4;
        auto size1 = cPageSize * cPagesCount1;
        auto size2 = cPageSize * cPagesCount2;
        auto size3 = cPageSize * cPagesCount3;
        auto memory1 = test::alignedAlloc(cPageSize, size1);
        auto memory2 = test::alignedAlloc(cPageSize, size2);
        auto memory3 = test::alignedAlloc(cPageSize, size3);

        constexpr int cRegionsCount = 4;
        std::array<Region, cRegionsCount> regions = {{{std::uintptr_t(memory1.get()), size1},
                                                      {std::uintptr_t(memory2.get()), size2},
                                                      {std::uintptr_t(memory3.get()), size3},
                                                      {0, 0}}};

        REQUIRE(pageAllocator.init(regions.data(), cPageSize));
        auto stats = pageAllocator.getStats();
        REQUIRE(stats.totalPagesCount == (cPagesCount1 + cPagesCount2 + cPagesCount3));
    }

    SECTION("All regions have 5 pages")
    {
        constexpr std::size_t cPagesCount = 5;
        auto size = cPageSize * cPagesCount;
        auto memory1 = test::alignedAlloc(cPageSize, size);
        auto memory2 = test::alignedAlloc(cPageSize, size);
        auto memory3 = test::alignedAlloc(cPageSize, size);
        auto memory4 = test::alignedAlloc(cPageSize, size);
        auto memory5 = test::alignedAlloc(cPageSize, size);
        auto memory6 = test::alignedAlloc(cPageSize, size);
        auto memory7 = test::alignedAlloc(cPageSize, size);
        auto memory8 = test::alignedAlloc(cPageSize, size);

        constexpr int cRegionsCount = 9;
        std::array<Region, cRegionsCount> regions = {{{std::uintptr_t(memory1.get()), size},
                                                      {std::uintptr_t(memory2.get()), size},
                                                      {std::uintptr_t(memory3.get()), size},
                                                      {std::uintptr_t(memory4.get()), size},
                                                      {std::uintptr_t(memory5.get()), size},
                                                      {std::uintptr_t(memory6.get()), size},
                                                      {std::uintptr_t(memory7.get()), size},
                                                      {std::uintptr_t(memory8.get()), size},
                                                      {0, 0}}};

        REQUIRE(pageAllocator.init(regions.data(), cPageSize));
        auto stats = pageAllocator.getStats();
        REQUIRE(stats.totalPagesCount == (cPagesCount * 8));
    }

    SECTION("All regions have 5 pages")
    {
        auto size = cPageSize / 2;
        auto memory1 = test::alignedAlloc(cPageSize, size);
        auto memory2 = test::alignedAlloc(cPageSize, size);
        auto memory3 = test::alignedAlloc(cPageSize, size);
        auto memory4 = test::alignedAlloc(cPageSize, size);
        auto memory5 = test::alignedAlloc(cPageSize, size);
        auto memory6 = test::alignedAlloc(cPageSize, size);
        auto memory7 = test::alignedAlloc(cPageSize, size);
        auto memory8 = test::alignedAlloc(cPageSize, size);

        constexpr int cRegionsCount = 9;
        std::array<Region, cRegionsCount> regions = {{{std::uintptr_t(memory1.get()), size},
                                                      {std::uintptr_t(memory2.get()), size},
                                                      {std::uintptr_t(memory3.get()), size},
                                                      {std::uintptr_t(memory4.get()), size},
                                                      {std::uintptr_t(memory5.get()), size},
                                                      {std::uintptr_t(memory6.get()), size},
                                                      {std::uintptr_t(memory7.get()), size},
                                                      {std::uintptr_t(memory8.get()), size},
                                                      {0, 0}}};

        REQUIRE(!pageAllocator.init(regions.data(), cPageSize));
        auto stats = pageAllocator.getStats();
        REQUIRE(stats.totalPagesCount == 0);
    }

    SECTION("Too many regions are passed to the allocator")
    {
        constexpr std::size_t cPagesCount = 5;
        auto size = cPageSize * cPagesCount;
        auto memory1 = test::alignedAlloc(cPageSize, size);
        auto memory2 = test::alignedAlloc(cPageSize, size);
        auto memory3 = test::alignedAlloc(cPageSize, size);
        auto memory4 = test::alignedAlloc(cPageSize, size);
        auto memory5 = test::alignedAlloc(cPageSize, size);
        auto memory6 = test::alignedAlloc(cPageSize, size);
        auto memory7 = test::alignedAlloc(cPageSize, size);
        auto memory8 = test::alignedAlloc(cPageSize, size);
        auto memory9 = test::alignedAlloc(cPageSize, size);
        auto memory10 = test::alignedAlloc(cPageSize, size);

        constexpr int cRegionsCount = 11;
        std::array<Region, cRegionsCount> regions = {{{std::uintptr_t(memory1.get()), size},
                                                      {std::uintptr_t(memory2.get()), size},
                                                      {std::uintptr_t(memory3.get()), size},
                                                      {std::uintptr_t(memory4.get()), size},
                                                      {std::uintptr_t(memory5.get()), size},
                                                      {std::uintptr_t(memory6.get()), size},
                                                      {std::uintptr_t(memory7.get()), size},
                                                      {std::uintptr_t(memory8.get()), size},
                                                      {std::uintptr_t(memory9.get()), size},
                                                      {std::uintptr_t(memory10.get()), size},
                                                      {0, 0}}};

        REQUIRE(!pageAllocator.init(regions.data(), cPageSize));
    }
}

TEST_CASE("Pages with page descriptors are properly reserved", "[unit][page_allocator]")
{
    constexpr std::size_t cPageSize = 256;
    PageAllocator pageAllocator;

    SECTION("Regions: 1(1)")
    {
        std::size_t pagesCount = 1;
        auto size = cPageSize * pagesCount;
        auto memory = test::alignedAlloc(cPageSize, size);

        constexpr int cRegionsCount = 2;
        std::array<Region, cRegionsCount> regions = {{{std::uintptr_t(memory.get()), size}, {0, 0}}};

        REQUIRE(pageAllocator.init(regions.data(), cPageSize));
        auto stats = pageAllocator.getStats();
        REQUIRE(stats.reservedPagesCount == 1);
    }

    SECTION("Regions: 1(535), 2(87), 3(4)")
    {
        constexpr std::size_t cPagesCount1 = 535;
        constexpr std::size_t cPagesCount2 = 87;
        constexpr std::size_t cPagesCount3 = 4;
        auto size1 = cPageSize * cPagesCount1;
        auto size2 = cPageSize * cPagesCount2;
        auto size3 = cPageSize * cPagesCount3;
        auto memory1 = test::alignedAlloc(cPageSize, size1);
        auto memory2 = test::alignedAlloc(cPageSize, size2);
        auto memory3 = test::alignedAlloc(cPageSize, size3);

        constexpr int cRegionsCount = 4;
        std::array<Region, cRegionsCount> regions = {{{std::uintptr_t(memory1.get()), size1},
                                                      {std::uintptr_t(memory2.get()), size2},
                                                      {std::uintptr_t(memory3.get()), size3},
                                                      {0, 0}}};

        REQUIRE(pageAllocator.init(regions.data(), cPageSize));
        auto stats = pageAllocator.getStats();
        REQUIRE(stats.reservedPagesCount == 79);
    }

    SECTION("All regions have 5 pages")
    {
        constexpr std::size_t cPagesCount = 5;
        auto size = cPageSize * cPagesCount;
        auto memory1 = test::alignedAlloc(cPageSize, size);
        auto memory2 = test::alignedAlloc(cPageSize, size);
        auto memory3 = test::alignedAlloc(cPageSize, size);
        auto memory4 = test::alignedAlloc(cPageSize, size);
        auto memory5 = test::alignedAlloc(cPageSize, size);
        auto memory6 = test::alignedAlloc(cPageSize, size);
        auto memory7 = test::alignedAlloc(cPageSize, size);
        auto memory8 = test::alignedAlloc(cPageSize, size);

        constexpr int cRegionsCount = 9;
        std::array<Region, cRegionsCount> regions = {{{std::uintptr_t(memory1.get()), size},
                                                      {std::uintptr_t(memory2.get()), size},
                                                      {std::uintptr_t(memory3.get()), size},
                                                      {std::uintptr_t(memory4.get()), size},
                                                      {std::uintptr_t(memory5.get()), size},
                                                      {std::uintptr_t(memory6.get()), size},
                                                      {std::uintptr_t(memory7.get()), size},
                                                      {std::uintptr_t(memory8.get()), size},
                                                      {0, 0}}};

        REQUIRE(pageAllocator.init(regions.data(), cPageSize));
        auto stats = pageAllocator.getStats();
        REQUIRE(stats.reservedPagesCount == 5);
    }

    SECTION("Selected region is completely filled")
    {
        constexpr std::size_t cPagesCount1 = 1;
        constexpr std::size_t cPagesCount2 = 7;
        auto size1 = cPageSize * cPagesCount1;
        auto size2 = cPageSize * cPagesCount2;
        auto memory1 = test::alignedAlloc(cPageSize, size1);
        auto memory2 = test::alignedAlloc(cPageSize, size2);

        constexpr int cRegionsCount = 3;
        std::array<Region, cRegionsCount> regions
            = {{{std::uintptr_t(memory1.get()), size1}, {std::uintptr_t(memory2.get()), size2}, {0, 0}}};

        REQUIRE(pageAllocator.init(regions.data(), cPageSize));
        auto stats = pageAllocator.getStats();
        REQUIRE(stats.reservedPagesCount == 1);
    }
}

TEST_CASE("Pages are correctly resolved from address", "[unit][page_allocator]")
{
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

    constexpr int cRegionsCount = 4;
    std::array<Region, cRegionsCount> regions = {{{std::uintptr_t(memory1.get()), size1},
                                                  {std::uintptr_t(memory2.get()), size2},
                                                  {std::uintptr_t(memory3.get()), size3},
                                                  {0, 0}}};

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

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
        REQUIRE(page->address() == (std::uintptr_t(memory1.get()) + (cPagesCount1 - 1) * cPageSize));
    }

    SECTION("Address points to the end of the second region")
    {
        page = pageAllocator.getPage(std::uintptr_t(memory2.get()) + size2 - 1);
        REQUIRE(page);
        REQUIRE(page->address() == (std::uintptr_t(memory2.get()) + (cPagesCount2 - 1) * cPageSize));
    }

    SECTION("Address points to the 16th page in the first region")
    {
        constexpr int cPageNum = 16;
        page = pageAllocator.getPage(std::uintptr_t(memory1.get()) + cPageNum * cPageSize);
        REQUIRE(page);
        REQUIRE(page->address() == (std::uintptr_t(memory1.get()) + cPageNum * cPageSize));
    }

    SECTION("Address points to the 7th page in the second region")
    {
        constexpr int cPageNum = 16;
        page = pageAllocator.getPage(std::uintptr_t(memory2.get()) + cPageNum * cPageSize);
        REQUIRE(page);
        REQUIRE(page->address() == (std::uintptr_t(memory2.get()) + cPageNum * cPageSize));
    }

    SECTION("Address points to in the middle of the second page in the first region")
    {
        page = pageAllocator.getPage(std::uintptr_t(memory1.get()) + cPageSize + cPageSize / 2);
        REQUIRE(page);
        REQUIRE(page->address() == (std::uintptr_t(memory1.get()) + cPageSize));
    }

    SECTION("Address points to in the middle of the third page in the third region")
    {
        page = pageAllocator.getPage(std::uintptr_t(memory3.get()) + 2 * cPageSize + cPageSize / 2);
        REQUIRE(page);
        REQUIRE(page->address() == (std::uintptr_t(memory3.get()) + 2 * cPageSize));
    }
}

TEST_CASE("PageAllocator stats are properly initialized", "[unit][page_allocator]")
{
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

    constexpr int cRegionsCount = 4;
    std::array<Region, cRegionsCount> regions = {{{std::uintptr_t(memory1.get()), size1},
                                                  {std::uintptr_t(memory2.get()), size2},
                                                  {std::uintptr_t(memory3.get()), size3},
                                                  {0, 0}}};

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    auto stats = pageAllocator.getStats();
    REQUIRE(stats.totalMemorySize == (size1 + size2 + size3));
    REQUIRE(stats.effectiveMemorySize == (size1 + size2 + size3));
    REQUIRE(stats.userMemorySize == stats.effectiveMemorySize - (stats.pageSize * stats.reservedPagesCount));
    REQUIRE(stats.freeMemorySize == (cPageSize * (stats.totalPagesCount - stats.reservedPagesCount)));
    REQUIRE(stats.pageSize == cPageSize);
    REQUIRE(stats.totalPagesCount == (cPagesCount1 + cPagesCount2 + cPagesCount3));
    REQUIRE(stats.reservedPagesCount == 79);
    REQUIRE(stats.freePagesCount == (stats.totalPagesCount - stats.reservedPagesCount));
}

TEST_CASE("Pages are correctly allocated", "[unit][page_allocator]")
{
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

    constexpr int cRegionsCount = 4;
    std::array<Region, cRegionsCount> regions = {{{std::uintptr_t(memory1.get()), size1},
                                                  {std::uintptr_t(memory2.get()), size2},
                                                  {std::uintptr_t(memory3.get()), size3},
                                                  {0, 0}}};

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));
    auto freePages = pageAllocator.getStats().freePagesCount;
    std::vector<Page*> pages;

    SECTION("Allocating 0 pages")
    {
        pages.push_back(pageAllocator.allocate(0));
        REQUIRE(pages.back() == nullptr);

        auto stats = pageAllocator.getStats();
        REQUIRE(stats.totalMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.effectiveMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.userMemorySize == stats.effectiveMemorySize - (stats.pageSize * stats.reservedPagesCount));
        REQUIRE(stats.freeMemorySize == (cPageSize * (stats.totalPagesCount - stats.reservedPagesCount)));
        REQUIRE(stats.pageSize == cPageSize);
        REQUIRE(stats.totalPagesCount == (cPagesCount1 + cPagesCount2 + cPagesCount3));
        REQUIRE(stats.reservedPagesCount == 79);
        REQUIRE(stats.freePagesCount == (stats.totalPagesCount - stats.reservedPagesCount));
    }

    SECTION("Allocating more pages than free count")
    {
        pages.push_back(pageAllocator.allocate(cPagesCount1 + cPagesCount2 + cPagesCount3 + 1));
        REQUIRE(pages.back() == nullptr);

        auto stats = pageAllocator.getStats();
        REQUIRE(stats.totalMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.effectiveMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.userMemorySize == stats.effectiveMemorySize - (stats.pageSize * stats.reservedPagesCount));
        REQUIRE(stats.freeMemorySize == (cPageSize * (stats.totalPagesCount - stats.reservedPagesCount)));
        REQUIRE(stats.pageSize == cPageSize);
        REQUIRE(stats.totalPagesCount == (cPagesCount1 + cPagesCount2 + cPagesCount3));
        REQUIRE(stats.reservedPagesCount == 79);
        REQUIRE(stats.freePagesCount == (stats.totalPagesCount - stats.reservedPagesCount));
    }

    SECTION("Allocating more pages than biggest free continues group")
    {
        pages.push_back(pageAllocator.allocate(cPagesCount1 + 1));
        REQUIRE(pages.back() == nullptr);

        auto stats = pageAllocator.getStats();
        REQUIRE(stats.totalMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.effectiveMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.userMemorySize == stats.effectiveMemorySize - (stats.pageSize * stats.reservedPagesCount));
        REQUIRE(stats.freeMemorySize == (cPageSize * (stats.totalPagesCount - stats.reservedPagesCount)));
        REQUIRE(stats.pageSize == cPageSize);
        REQUIRE(stats.totalPagesCount == (cPagesCount1 + cPagesCount2 + cPagesCount3));
        REQUIRE(stats.reservedPagesCount == 79);
        REQUIRE(stats.freePagesCount == (stats.totalPagesCount - stats.reservedPagesCount));
    }

    SECTION("Allocating 1 page")
    {
        pages.push_back(pageAllocator.allocate(1));
        REQUIRE(pages.back());
        REQUIRE(pages.back()->address() == std::uintptr_t(memory3.get()));

        auto stats = pageAllocator.getStats();
        REQUIRE(stats.totalMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.effectiveMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.userMemorySize == stats.effectiveMemorySize - (stats.pageSize * stats.reservedPagesCount));
        REQUIRE(stats.freeMemorySize
                == (cPageSize * (stats.totalPagesCount - stats.reservedPagesCount) - 1 * cPageSize));
        REQUIRE(stats.pageSize == cPageSize);
        REQUIRE(stats.totalPagesCount == (cPagesCount1 + cPagesCount2 + cPagesCount3));
        REQUIRE(stats.reservedPagesCount == 79);
        REQUIRE(stats.freePagesCount == (stats.totalPagesCount - stats.reservedPagesCount - 1));
    }

    SECTION("Allocating 17 pages")
    {
        constexpr int cAllocSize = 17;
        pages.push_back(pageAllocator.allocate(cAllocSize));
        REQUIRE(pages.back());
        REQUIRE(pages.back()->address() == std::uintptr_t(memory1.get()));

        auto stats = pageAllocator.getStats();
        REQUIRE(stats.totalMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.effectiveMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.userMemorySize == stats.effectiveMemorySize - (stats.pageSize * stats.reservedPagesCount));
        REQUIRE(stats.freeMemorySize
                == (cPageSize * (stats.totalPagesCount - stats.reservedPagesCount) - cAllocSize * cPageSize));
        REQUIRE(stats.pageSize == cPageSize);
        REQUIRE(stats.totalPagesCount == (cPagesCount1 + cPagesCount2 + cPagesCount3));
        REQUIRE(stats.reservedPagesCount == 79);
        REQUIRE(stats.freePagesCount == (stats.totalPagesCount - stats.reservedPagesCount - cAllocSize));
    }

    SECTION("Allocating whole region")
    {
        pages.push_back(pageAllocator.allocate(cPagesCount1));
        REQUIRE(pages.back());
        REQUIRE(pages.back()->address() == std::uintptr_t(memory1.get()));

        auto stats = pageAllocator.getStats();
        REQUIRE(stats.totalMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.effectiveMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.userMemorySize == stats.effectiveMemorySize - (stats.pageSize * stats.reservedPagesCount));
        REQUIRE(stats.freeMemorySize
                == (cPageSize * (stats.totalPagesCount - stats.reservedPagesCount) - cPagesCount1 * cPageSize));
        REQUIRE(stats.pageSize == cPageSize);
        REQUIRE(stats.totalPagesCount == (cPagesCount1 + cPagesCount2 + cPagesCount3));
        REQUIRE(stats.reservedPagesCount == 79);
        REQUIRE(stats.freePagesCount == (stats.totalPagesCount - stats.reservedPagesCount - cPagesCount1));
    }

    SECTION("Allocate 1 page 4 times")
    {
        for (int i = 0; i < 4; ++i) {
            pages.push_back(pageAllocator.allocate(1));
            REQUIRE(pages[i]);
            REQUIRE(pages[i]->address() == std::uintptr_t(memory3.get()) + i * cPageSize);
            REQUIRE(pageAllocator.getStats().freePagesCount == freePages - i - 1);
        }

        auto stats = pageAllocator.getStats();
        REQUIRE(stats.totalMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.effectiveMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.userMemorySize == stats.effectiveMemorySize - (stats.pageSize * stats.reservedPagesCount));
        REQUIRE(stats.freeMemorySize
                == (cPageSize * (stats.totalPagesCount - stats.reservedPagesCount) - 4 * cPageSize));
        REQUIRE(stats.pageSize == cPageSize);
        REQUIRE(stats.totalPagesCount == (cPagesCount1 + cPagesCount2 + cPagesCount3));
        REQUIRE(stats.reservedPagesCount == 79);
        REQUIRE(stats.freePagesCount == (stats.totalPagesCount - stats.reservedPagesCount - 4));
    }

    SECTION("Only 2 pages are left in each region")
    {
        std::size_t allocated = 0;

        pages.push_back(pageAllocator.allocate(cPagesCount3 - 2));
        allocated += cPagesCount3 - 2;
        REQUIRE(pages.back());
        REQUIRE(pages.back()->address() == std::uintptr_t(memory3.get()));
        REQUIRE(pageAllocator.getStats().freePagesCount == freePages - allocated);

        pages.push_back(pageAllocator.allocate(cPagesCount2 - pageAllocator.getStats().reservedPagesCount - 2));
        allocated += cPagesCount2 - pageAllocator.getStats().reservedPagesCount - 2;
        REQUIRE(pages.back());
        REQUIRE(pages.back()->address()
                == std::uintptr_t(memory2.get() + pageAllocator.getStats().reservedPagesCount * cPageSize));
        REQUIRE(pageAllocator.getStats().freePagesCount == freePages - 8);

        pages.push_back(pageAllocator.allocate(cPagesCount1 - 2));
        allocated += cPagesCount1 - 2;
        REQUIRE(pages.back());
        REQUIRE(pages.back()->address() == std::uintptr_t(memory1.get()));
        REQUIRE(pageAllocator.getStats().freePagesCount == freePages - allocated);

        auto stats = pageAllocator.getStats();
        REQUIRE(stats.totalMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.effectiveMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.userMemorySize == stats.effectiveMemorySize - (stats.pageSize * stats.reservedPagesCount));
        REQUIRE(stats.freeMemorySize == 6 * cPageSize);
        REQUIRE(stats.pageSize == cPageSize);
        REQUIRE(stats.totalPagesCount == (cPagesCount1 + cPagesCount2 + cPagesCount3));
        REQUIRE(stats.reservedPagesCount == 79);
        REQUIRE(stats.freePagesCount == 6);
    }

    SECTION("Allocate all pages one by one")
    {
        for (std::size_t i = 0; i < freePages; ++i) {
            pages.push_back(pageAllocator.allocate(1));
            REQUIRE(pages.back());
            REQUIRE(pageAllocator.getStats().freePagesCount == freePages - i - 1);
            REQUIRE(pageAllocator.getPage(pages.back()->address()) == pages[i]);
        }

        auto stats = pageAllocator.getStats();
        REQUIRE(stats.totalMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.effectiveMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.userMemorySize == stats.effectiveMemorySize - (stats.pageSize * stats.reservedPagesCount));
        REQUIRE(stats.freeMemorySize == 0);
        REQUIRE(stats.pageSize == cPageSize);
        REQUIRE(stats.totalPagesCount == (cPagesCount1 + cPagesCount2 + cPagesCount3));
        REQUIRE(stats.reservedPagesCount == 79);
        REQUIRE(stats.freePagesCount == 0);
    }
}

TEST_CASE("Pages are correctly released", "[unit][page_allocator]")
{
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

    constexpr int cRegionsCount = 4;
    std::array<Region, cRegionsCount> regions = {{{std::uintptr_t(memory1.get()), size1},
                                                  {std::uintptr_t(memory2.get()), size2},
                                                  {std::uintptr_t(memory3.get()), size3},
                                                  {0, 0}}};

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));
    auto freePages = pageAllocator.getStats().freePagesCount;
    std::vector<Page*> pages;

    // clang-format off
    SECTION("Releasing nullptr is a valid operation")
    {
        pageAllocator.release(nullptr);
    }
    // clang-format on

    SECTION("Releasing 1 page")
    {
        pages.push_back(pageAllocator.allocate(1));
        pageAllocator.release(pages.back());
    }

    SECTION("Releasing 17 pages")
    {
        constexpr int cAllocSize = 17;
        pages.push_back(pageAllocator.allocate(cAllocSize));
        pageAllocator.release(pages.back());
    }

    SECTION("Releasing whole region")
    {
        pages.push_back(pageAllocator.allocate(cPagesCount1));
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
        pages.push_back(pageAllocator.allocate(cPagesCount3 - 2));
        pages.push_back(pageAllocator.allocate(cPagesCount2 - pageAllocator.getStats().reservedPagesCount - 2));
        pages.push_back(pageAllocator.allocate(cPagesCount1 - 2));

        for (auto* page : pages)
            pageAllocator.release(page);
    }

    SECTION("Only 2 pages are left in each region, release from last")
    {
        pages.push_back(pageAllocator.allocate(cPagesCount3 - 2));
        pages.push_back(pageAllocator.allocate(cPagesCount2 - pageAllocator.getStats().reservedPagesCount - 2));
        pages.push_back(pageAllocator.allocate(cPagesCount1 - 2));

        for (std::size_t i = 0; i < pages.size(); ++i)
            pageAllocator.release(pages[pages.size() - 1 - i]);
    }

    SECTION("Allocate all pages one by one, release from first")
    {
        for (std::size_t i = 0; i < freePages; ++i)
            pages.push_back(pageAllocator.allocate(1));

        for (auto* page : pages)
            pageAllocator.release(page);
    }

    SECTION("Allocate all pages one by one, release from first")
    {
        for (std::size_t i = 0; i < freePages; ++i)
            pages.push_back(pageAllocator.allocate(1));

        for (std::size_t i = 0; i < pages.size(); ++i)
            pageAllocator.release(pages[pages.size() - 1 - i]);
    }

    auto stats = pageAllocator.getStats();
    REQUIRE(stats.totalMemorySize == (size1 + size2 + size3));
    REQUIRE(stats.effectiveMemorySize == (size1 + size2 + size3));
    REQUIRE(stats.userMemorySize == stats.effectiveMemorySize - (stats.pageSize * stats.reservedPagesCount));
    REQUIRE(stats.freeMemorySize == (cPageSize * (stats.totalPagesCount - stats.reservedPagesCount)));
    REQUIRE(stats.pageSize == cPageSize);
    REQUIRE(stats.totalPagesCount == (cPagesCount1 + cPagesCount2 + cPagesCount3));
    REQUIRE(stats.reservedPagesCount == 79);
    REQUIRE(stats.freePagesCount == (stats.totalPagesCount - stats.reservedPagesCount));
    REQUIRE(stats.freePagesCount == freePages);
}

} // namespace memory
