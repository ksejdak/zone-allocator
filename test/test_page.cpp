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

// Make access to private members for testing.
// clang-format off
#define private     public
// clang-format on

#include <page.h>

#include <cstddef>

using namespace Memory;

TEST_CASE("Page structure is naturally aligned", "[page]")
{
    std::size_t requiredSize = 0;
    requiredSize += sizeof(Page*);          // m_nextGroup
    requiredSize += sizeof(Page*);          // m_prevGroup
    requiredSize += sizeof(std::uintptr_t); // m_addr
    requiredSize += sizeof(Page::Flags);    // m_flags

    REQUIRE(sizeof(Page) == requiredSize);
}

TEST_CASE("Page is properly initialized", "[page]")
{
    std::byte buffer[sizeof(Page)];
    auto* page = reinterpret_cast<Page*>(buffer);

    page->init();
    REQUIRE(page->m_nextGroup == nullptr);
    REQUIRE(page->m_prevGroup == nullptr);
    REQUIRE(page->m_addr == 0);
    REQUIRE(page->m_flags.value == 0);
}

TEST_CASE("Accessing siblings works as expected", "[page]")
{
    constexpr int pageCount = 3;
    std::byte buffer[pageCount * sizeof(Page)];
    Page* page[pageCount];

    for (int i = 0; i < pageCount; ++i) {
        page[i] = reinterpret_cast<Page*>(buffer) + i;
        page[i]->init();
        page[i]->setAddress(std::uintptr_t(i));
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

TEST_CASE("Adding to empty list", "[page]")
{
    std::byte buffer[sizeof(Page)];
    auto* page = reinterpret_cast<Page*>(buffer);

    page->init();
    page->setAddress(1);

    Page* list = nullptr;
    page->addToList(&list);
    REQUIRE(list == page);
    REQUIRE(list->address() == 1);
    REQUIRE(page->m_nextGroup == nullptr);
    REQUIRE(page->m_prevGroup == nullptr);
}

TEST_CASE("Adding to non-empty list", "[page]")
{
    constexpr int pageCount = 5;
    std::byte buffer[pageCount * sizeof(Page)];
    Page* page[pageCount];

    Page* list = nullptr;
    for (int i = 0; i < pageCount; ++i) {
        page[i] = reinterpret_cast<Page*>(buffer) + i;
        page[i]->init();
        page[i]->setAddress(std::uintptr_t(i));
        page[i]->addToList(&list);
    }

    SECTION("All pages are in the list")
    {
        bool pagePresent[pageCount] = {};

        for (auto* it = list; it != nullptr; it = it->nextGroup()) {
            auto idx = it->address();
            REQUIRE(idx < pageCount);
            pagePresent[idx] = true;
        }

        for (auto& present : pagePresent)
            REQUIRE(present);
    }

    SECTION("All pages are in correct order")
    {
        int idx = pageCount - 1;
        for (auto *it = list; it != nullptr; it = it->nextGroup(), --idx)
            REQUIRE(it->address() == page[idx]->address());
    }
}

TEST_CASE("Removing from list with 5 pages", "[page]")
{
    constexpr int pageCount = 5;
    std::byte buffer[pageCount * sizeof(Page)];
    Page* page[pageCount];

    Page* list = nullptr;
    for (int i = 0; i < pageCount; ++i) {
        page[i] = reinterpret_cast<Page*>(buffer) + i;
        page[i]->init();
        page[i]->setAddress(std::uintptr_t(i));
        page[i]->addToList(&list);
    }

    SECTION("Removing first page from list")
    {
        int idx = 4;
        page[idx]->removeFromList(&list);
        REQUIRE(page[idx]->address() == idx);
        REQUIRE(page[idx]->m_nextGroup == nullptr);
        REQUIRE(page[idx]->m_prevGroup == nullptr);

        int i = pageCount - 1;
        for (auto *it = list; it != nullptr; it = it->nextGroup(), --i) {
            if (i == idx)
                --i;

            REQUIRE(it->address() == page[i]->address());
        }
    }

    SECTION("Removing middle page from list")
    {
        int idx = 2;
        page[idx]->removeFromList(&list);
        REQUIRE(page[idx]->address() == idx);
        REQUIRE(page[idx]->m_nextGroup == nullptr);
        REQUIRE(page[idx]->m_prevGroup == nullptr);

        int i = pageCount - 1;
        for (auto *it = list; it != nullptr; it = it->nextGroup(), --i) {
            if (i == idx)
                --i;

            REQUIRE(it->address() == page[i]->address());
        }
    }

    SECTION("Removing last page from list")
    {
        int idx = 0;
        page[idx]->removeFromList(&list);
        REQUIRE(page[idx]->address() == idx);
        REQUIRE(page[idx]->m_nextGroup == nullptr);
        REQUIRE(page[idx]->m_prevGroup == nullptr);

        int i = pageCount - 1;
        for (auto *it = list; it != nullptr; it = it->nextGroup(), --i) {
            if (i == idx)
                --i;

            REQUIRE(it->address() == page[i]->address());
        }
    }

    SECTION("Removing all pages starting from first")
    {
        for (int i = pageCount - 1; i >= 0; --i) {
            page[i]->removeFromList(&list);
            REQUIRE(page[i]->address() == i);
            REQUIRE(page[i]->m_nextGroup == nullptr);
            REQUIRE(page[i]->m_prevGroup == nullptr);
        }

        REQUIRE(list == nullptr);
    }

    SECTION("Removing all pages starting from last")
    {
        for (int i = 0; i < pageCount; ++i) {
            page[i]->removeFromList(&list);
            REQUIRE(page[i]->address() == i);
            REQUIRE(page[i]->m_nextGroup == nullptr);
            REQUIRE(page[i]->m_prevGroup == nullptr);
        }

        REQUIRE(list == nullptr);
    }
}

TEST_CASE("Removing from list with 1 page", "[page]")
{
    std::byte buffer[sizeof(Page)];
    auto* page = reinterpret_cast<Page*>(buffer);

    page->init();
    page->setAddress(1);

    Page* list = nullptr;
    page->addToList(&list);

    page->removeFromList(&list);
    REQUIRE(list == nullptr);
    REQUIRE(page->address() == 1);
    REQUIRE(page->m_nextGroup == nullptr);
    REQUIRE(page->m_prevGroup == nullptr);
}
