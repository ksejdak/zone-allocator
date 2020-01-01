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

#include <group.hpp>
#include <page.hpp>

#include <catch2/catch.hpp>

#include <array>
#include <map>
#include <utility>

namespace memory {

TEST_CASE("Group index is properly computed", "[unit][group]")
{
    std::map<std::size_t, std::pair<std::size_t, size_t>> idxRange = {{0, {0, 3}},               // NOLINT
                                                                      {1, {4, 7}},               // NOLINT
                                                                      {2, {8, 15}},              // NOLINT
                                                                      {3, {16, 31}},             // NOLINT
                                                                      {4, {32, 63}},             // NOLINT
                                                                      {5, {64, 127}},            // NOLINT
                                                                      {6, {128, 255}},           // NOLINT
                                                                      {7, {256, 511}},           // NOLINT
                                                                      {8, {512, 1023}},          // NOLINT
                                                                      {9, {1024, 2047}},         // NOLINT
                                                                      {10, {2048, 4095}},        // NOLINT
                                                                      {11, {4096, 8191}},        // NOLINT
                                                                      {12, {8192, 16383}},       // NOLINT
                                                                      {13, {16384, 32767}},      // NOLINT
                                                                      {14, {32768, 65535}},      // NOLINT
                                                                      {15, {65536, 131071}},     // NOLINT
                                                                      {16, {131072, 262143}},    // NOLINT
                                                                      {17, {262144, 524287}},    // NOLINT
                                                                      {18, {524288, 1048575}},   // NOLINT
                                                                      {19, {1048576, 2097151}}}; // NOLINT

    constexpr std::size_t cIterations = 0x200000;
    for (std::size_t i = 0; i < cIterations; ++i) {
        auto idx = groupIdx(i);
        REQUIRE(i >= idxRange[idx].first);
        REQUIRE(i <= idxRange[idx].second);
    }
}

TEST_CASE("Group is properly initialized", "[unit][group]")
{
    SECTION("Group has 1 page")
    {
        constexpr std::size_t cGroupSize = 1;
        std::array<std::byte, sizeof(Page) * cGroupSize> memory{};

        auto* group = reinterpret_cast<Page*>(std::begin(memory));
        initGroup(group, cGroupSize);
        REQUIRE(group->groupSize() == cGroupSize);
    }

    SECTION("Group has 5 pages")
    {
        constexpr std::size_t cGroupSize = 5;
        std::array<std::byte, sizeof(Page) * cGroupSize> memory{};

        auto* group = reinterpret_cast<Page*>(std::begin(memory));
        initGroup(group, cGroupSize);
        Page* firstPage = group;
        Page* lastPage = group + cGroupSize - 1;
        REQUIRE(firstPage->groupSize() == cGroupSize);
        REQUIRE(lastPage->groupSize() == cGroupSize);
    }
}

TEST_CASE("Group is properly cleared", "[unit][group]")
{
    SECTION("Group has 1 page")
    {
        constexpr std::size_t cGroupSize = 1;
        std::array<std::byte, sizeof(Page) * cGroupSize> memory{};

        auto* group = reinterpret_cast<Page*>(std::begin(memory));
        initGroup(group, cGroupSize);
        clearGroup(group);
        REQUIRE(group->groupSize() == 0);
    }

    SECTION("Group has 5 pages")
    {
        constexpr std::size_t cGroupSize = 5;
        std::array<std::byte, sizeof(Page) * cGroupSize> memory{};

        auto* group = reinterpret_cast<Page*>(std::begin(memory));
        initGroup(group, cGroupSize);
        clearGroup(group);
        Page* firstPage = group;
        Page* lastPage = group + cGroupSize - 1;
        REQUIRE(firstPage->groupSize() == 0);
        REQUIRE(lastPage->groupSize() == 0);
    }
}

TEST_CASE("Group is properly splitted", "[unit][group]")
{
    constexpr std::size_t cGroupSize = 10;
    std::array<std::byte, sizeof(Page) * cGroupSize> memory{};

    auto* group = reinterpret_cast<Page*>(std::begin(memory));

    initGroup(group, cGroupSize);

    Page* firstGroup = nullptr;
    Page* secondGroup = nullptr;
    std::size_t splitSize = 0;

    SECTION("Split size is equal to group size")
    {
        splitSize = cGroupSize;
        std::tie(firstGroup, secondGroup) = splitGroup(group, splitSize);
        REQUIRE(firstGroup);
        REQUIRE(firstGroup->groupSize() == splitSize);
        REQUIRE(secondGroup == nullptr);
    }

    SECTION("First group should have 1 page")
    {
        splitSize = 1;
        std::tie(firstGroup, secondGroup) = splitGroup(group, splitSize);
        REQUIRE(firstGroup);
        REQUIRE(secondGroup);
        REQUIRE(firstGroup->groupSize() == splitSize);
        REQUIRE(secondGroup->groupSize() == cGroupSize - splitSize);
    }

    SECTION("First group should have 3 pages")
    {
        splitSize = 3;
        std::tie(firstGroup, secondGroup) = splitGroup(group, splitSize);
        REQUIRE(firstGroup);
        REQUIRE(secondGroup);
        REQUIRE(firstGroup->groupSize() == splitSize);
        REQUIRE(secondGroup->groupSize() == cGroupSize - splitSize);
    }

    SECTION("First group should have 5 pages")
    {
        splitSize = 5; // NOLINT
        std::tie(firstGroup, secondGroup) = splitGroup(group, splitSize);
        REQUIRE(firstGroup);
        REQUIRE(secondGroup);
        REQUIRE(firstGroup->groupSize() == splitSize);
        REQUIRE(secondGroup->groupSize() == cGroupSize - splitSize);
    }
}

TEST_CASE("Group is properly joined", "[unit][group]")
{
    constexpr std::size_t cGroupSize = 10;
    std::array<std::byte, sizeof(Page) * cGroupSize> memory{};

    auto* group = reinterpret_cast<Page*>(std::begin(memory));

    initGroup(group, cGroupSize);

    std::size_t splitSize = 0;

    // clang-format off
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
        splitSize = 5; // NOLINT
    }
    // clang-format on

    Page* firstGroup = nullptr;
    Page* secondGroup = nullptr;
    std::tie(firstGroup, secondGroup) = splitGroup(group, splitSize);
    Page* joinedGroup = joinGroup(firstGroup, secondGroup);
    REQUIRE(joinedGroup);
    REQUIRE(joinedGroup->groupSize() == cGroupSize);
}

} // namespace memory
