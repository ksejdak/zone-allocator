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

#include <list_node.hpp>

#include <catch2/catch.hpp>

#include <array>
#include <cstddef>

namespace memory {

struct TestNode : public ListNode<TestNode> {
    TestNode() = default;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    int value{};
};

TEST_CASE("List node is properly initialized", "[unit][list_node]")
{
    TestNode node;
    node.initListNode();
    REQUIRE(node.next() == nullptr);
    REQUIRE(node.prev() == nullptr);
}

TEST_CASE("Adding to empty list", "[unit][list_node]")
{
    TestNode node;
    node.initListNode();
    node.value = 1;

    TestNode* list = nullptr;
    node.addToList(&list);
    REQUIRE(list == &node);
    REQUIRE(list->value == 1);
    REQUIRE(node.next() == nullptr);
    REQUIRE(node.prev() == nullptr);
}

TEST_CASE("Adding to non-empty list", "[unit][list_node]")
{
    constexpr int cNodeCount = 5;
    std::array<TestNode, cNodeCount> node{};

    TestNode* list = nullptr;
    for (int i = 0; i < cNodeCount; ++i) {
        node.at(i).initListNode();
        node.at(i).value = i;
        node.at(i).addToList(&list);
    }

    SECTION("All nodes are in the list")
    {
        std::array<bool, cNodeCount> nodePresent{};

        for (auto* it = list; it != nullptr; it = it->next()) {
            auto idx = it->value;
            REQUIRE(idx < cNodeCount);
            nodePresent.at(idx) = true;
        }

        for (auto& present : nodePresent)
            REQUIRE(present);
    }

    SECTION("All nodes are in correct order")
    {
        int idx = cNodeCount - 1;
        for (auto* it = list; it != nullptr; it = it->next(), --idx)
            REQUIRE(it->value == node.at(idx).value);
    }
}

TEST_CASE("Removing from list with 5 nodes", "[unit][list_node]")
{
    constexpr int cNodeCount = 5;
    std::array<TestNode, cNodeCount> node{};

    TestNode* list = nullptr;
    for (int i = 0; i < cNodeCount; ++i) {
        node.at(i).initListNode();
        node.at(i).value = i;
        node.at(i).addToList(&list);
    }

    SECTION("Removing first node from list")
    {
        int idx = 4;
        node.at(idx).removeFromList(&list);
        REQUIRE(node.at(idx).value == idx);
        REQUIRE(node.at(idx).next() == nullptr);
        REQUIRE(node.at(idx).prev() == nullptr);

        int i = cNodeCount - 1;
        for (auto* it = list; it != nullptr; it = it->next(), --i) {
            if (i == idx)
                --i;

            REQUIRE(it->value == node.at(i).value);
        }
    }

    SECTION("Removing middle node from list")
    {
        int idx = 2;
        node.at(idx).removeFromList(&list);
        REQUIRE(node.at(idx).value == idx);
        REQUIRE(node.at(idx).next() == nullptr);
        REQUIRE(node.at(idx).prev() == nullptr);

        int i = cNodeCount - 1;
        for (auto* it = list; it != nullptr; it = it->next(), --i) {
            if (i == idx)
                --i;

            REQUIRE(it->value == node.at(i).value);
        }
    }

    SECTION("Removing last node from list")
    {
        int idx = 0;
        node.at(idx).removeFromList(&list);
        REQUIRE(node.at(idx).value == idx);
        REQUIRE(node.at(idx).next() == nullptr);
        REQUIRE(node.at(idx).prev() == nullptr);

        int i = cNodeCount - 1;
        for (auto* it = list; it != nullptr; it = it->next(), --i) {
            if (i == idx)
                --i;

            REQUIRE(it->value == node.at(i).value);
        }
    }

    SECTION("Removing all nodes starting from first")
    {
        for (int i = cNodeCount - 1; i >= 0; --i) {
            node.at(i).removeFromList(&list);
            REQUIRE(node.at(i).value == i);
            REQUIRE(node.at(i).next() == nullptr);
            REQUIRE(node.at(i).prev() == nullptr);
        }

        REQUIRE(list == nullptr);
    }

    SECTION("Removing all nodes starting from last")
    {
        for (int i = 0; i < cNodeCount; ++i) {
            node.at(i).removeFromList(&list);
            REQUIRE(node.at(i).value == i);
            REQUIRE(node.at(i).next() == nullptr);
            REQUIRE(node.at(i).prev() == nullptr);
        }

        REQUIRE(list == nullptr);
    }
}

TEST_CASE("Removing from list with 1 node", "[unit][list_node]")
{
    TestNode node;
    node.initListNode();
    node.value = 1;

    TestNode* list = nullptr;
    node.addToList(&list);

    node.removeFromList(&list);
    REQUIRE(list == nullptr);
    REQUIRE(node.value == 1);
    REQUIRE(node.next() == nullptr);
    REQUIRE(node.prev() == nullptr);
}

} // namespace memory
