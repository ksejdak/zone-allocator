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

#include <page.hpp>

#include <catch2/catch.hpp>

#include <array>
#include <cstddef>

namespace memory {

TEST_CASE("Page structure is naturally aligned", "[unit][page]")
{
    REQUIRE(Page::isNaturallyAligned());
}

TEST_CASE("Page is properly initialized", "[unit][page]")
{
    std::array<std::byte, sizeof(Page)> buffer{};
    auto* page = reinterpret_cast<Page*>(buffer.data());

    page->init();
    REQUIRE(page->next() == nullptr);
    REQUIRE(page->prev() == nullptr);
    REQUIRE(page->address() == 0);
    REQUIRE(page->groupSize() == 0);
    REQUIRE(!page->isUsed());
}

TEST_CASE("Accessing siblings works as expected", "[unit][page]")
{
    constexpr int cPageCount = 3;
    std::array<std::byte, cPageCount * sizeof(Page)> buffer{};
    std::array<Page*, cPageCount> page{};

    for (int i = 0; i < cPageCount; ++i) {
        page.at(i) = reinterpret_cast<Page*>(buffer.data()) + i;
        page.at(i)->init();
        page.at(i)->setAddress(std::uintptr_t(i));
    }

    SECTION("Previous sibling")
    {
        auto* prev = page[1]->prevSibling();
        REQUIRE(prev->address() == page[0]->address());
    }

    SECTION("Next sibling")
    {
        auto* next = page[1]->nextSibling();
        REQUIRE(next->address() == page[2]->address());
    }
}

} // namespace memory
