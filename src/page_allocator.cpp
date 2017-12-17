////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include "page_allocator.h"

#include <cmath>

namespace Memory {

PageAllocator::PageAllocator()
{
    clear();
}

bool PageAllocator::init(Region* regions)
{
    auto descRegionIdx = chooseDescRegion(regions);
    if (!descRegionIdx)
        return false;

    m_pagesHead = reinterpret_cast<Page*>(regions[*descRegionIdx].address);

    for (auto* region = regions; region->size != 0; ++region) {
        auto* page = reinterpret_cast<Page*>(regions->address);

        for (auto addr = alignedStart(region); addr != alignedEnd(region); addr += PAGE_SIZE) {
            page->init();
            page->setAddress(addr);

            if (page != m_pagesHead) {
                page->prevSibling()->setNext(page);
                page->setPrev(page->prevSibling());
            }

            m_pagesTail = page;
            ++m_pagesCount;
            ++page;
        }
    }

    reserveDescPages();
    return true;
}

void PageAllocator::clear()
{
    m_pagesHead = nullptr;
    m_pagesTail = nullptr;
    m_freeGroups.fill(nullptr);
    m_pagesCount = 0;
}

std::optional<int> PageAllocator::chooseDescRegion(Region* regions)
{
    int pagesCount = countPages(regions);
    if (!pagesCount)
        return std::nullopt;

    int descAreaSize = pagesCount * sizeof(Page);

    int selectedIdx = 0;
    for (int i = 0; regions[i].size != 0; ++i) {
        if (regions[i].size < descAreaSize)
            continue;

        if (regions[i].size < regions[selectedIdx].size)
            selectedIdx = i;
    }

    return std::make_optional(selectedIdx);
}

int PageAllocator::countPages(Region* regions)
{
    int pagesCount = 0;
    for (auto* region = regions; region->size != 0; ++region)
        pagesCount += (alignedEnd(region) - alignedStart(region)) / PAGE_SIZE;

    return pagesCount;
}

void PageAllocator::reserveDescPages()
{
    for (auto* page = m_pagesHead; page->address() <= m_pagesTail->address(); page = page->nextSibling())
        page->setUsed(true);
}

std::uintptr_t PageAllocator::alignedStart(Region* region)
{
    auto start = region->address & ~(PAGE_SIZE - 1);
    if (start < region->address)
        start += PAGE_SIZE;

    return start;
}

std::uintptr_t PageAllocator::alignedEnd(Region* region)
{
    return (region->address + region->size) & ~(PAGE_SIZE - 1);
}

int PageAllocator::groupIdx(int pageCount)
{
    return static_cast<int>(ceil(log2(pageCount)) - 1);
}

} // namespace Memory
