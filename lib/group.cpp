/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2017-2019, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include "group.hpp"

#include "page.hpp"

#include <cassert>
#include <cmath>

namespace memory {

std::size_t groupIdx(std::size_t pageCount)
{
    if (pageCount < 2)
        return 0;

    return static_cast<std::size_t>(std::floor(std::log2(pageCount)) - 1);
}

void initGroup(Page* group, std::size_t groupSize)
{
    assert(group);

    Page* firstPage = group;
    Page* lastPage = group + groupSize - 1;
    firstPage->setGroupSize(groupSize);
    lastPage->setGroupSize(groupSize);
}

void clearGroup(Page* group)
{
    assert(group);

    Page* firstPage = group;
    Page* lastPage = group + group->groupSize() - 1;
    firstPage->setGroupSize(0);
    lastPage->setGroupSize(0);
}

std::tuple<Page*, Page*> splitGroup(Page* group, std::size_t size)
{
    assert(group);
    assert(size);
    auto groupSize = group->groupSize();
    assert(size <= groupSize);

    if (size == groupSize)
        return std::tuple<Page*, Page*>(group, nullptr);

    std::size_t secondSize = groupSize - size;
    clearGroup(group);

    Page* firstGroup = group;
    Page* secondGroup = group + size;

    initGroup(firstGroup, size);
    initGroup(secondGroup, secondSize);

    return std::make_tuple(firstGroup, secondGroup);
}

Page* joinGroup(Page* firstGroup, Page* secondGroup)
{
    assert(firstGroup);
    assert(secondGroup);

    std::size_t joinedSize = firstGroup->groupSize() + secondGroup->groupSize();

    clearGroup(firstGroup);
    clearGroup(secondGroup);
    initGroup(firstGroup, joinedSize);

    return firstGroup;
}

} // namespace memory
