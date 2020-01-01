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

#include "page_allocator.hpp"

#include "group.hpp"
#include "page.hpp"

#include <allocator/region.hpp>

#include <cassert>
#include <cmath>
#include <numeric>

namespace memory {

PageAllocator::PageAllocator() noexcept
{
    clear();
}

bool PageAllocator::init(Region* regions, std::size_t pageSize)
{
    assert(regions);

    clear();

    if (!detail::isValidPageSize(pageSize))
        return false;

    for (std::size_t i = 0; regions[i].size != 0; ++i) {
        if (i == cMaxRegionsCount)
            return false;

        RegionInfo regionInfo{};
        if (initRegionInfo(regionInfo, regions[i], pageSize))
            m_regionsInfo.at(m_validRegionsCount++) = regionInfo;
    }

    if ((m_pagesCount = countPages()) == 0)
        return false;

    m_pageSize = pageSize;
    m_descRegionIdx = chooseDescRegion();
    m_pagesHead = reinterpret_cast<Page*>(m_regionsInfo.at(m_descRegionIdx).alignedStart);
    m_pagesTail = m_pagesHead + m_pagesCount - 1;

    auto* page = m_pagesHead;
    for (std::size_t i = 0; i < m_validRegionsCount; ++i) {
        auto& region = m_regionsInfo.at(i);

        region.firstPage = page;
        region.lastPage = page + region.pageCount - 1;

        for (auto addr = region.alignedStart; addr != region.alignedEnd; addr += m_pageSize) {
            assert(page);
            page->init();
            page->setAddress(addr);
            page = page->nextSibling();
        }

        Page* group = region.firstPage;
        initGroup(group, region.pageCount);

        if (i == m_descRegionIdx) {
            m_descPagesCount = reserveDescPages();
            std::tie(std::ignore, group) = splitGroup(group, m_descPagesCount);
        }

        if (group != nullptr)
            addGroup(group);
    }

    return true;
}

void PageAllocator::clear()
{
    for (auto& region : m_regionsInfo)
        clearRegionInfo(region);

    m_validRegionsCount = 0;
    m_pageSize = 0;
    m_descRegionIdx = 0;
    m_descPagesCount = 0;
    m_pagesHead = nullptr;
    m_pagesTail = nullptr;
    m_freeGroupLists.fill(nullptr);
    m_pagesCount = 0;
    m_freePagesCount = 0;
}

Page* PageAllocator::allocate(std::size_t count)
{
    if (m_freePagesCount < count || count == 0)
        return nullptr;

    std::size_t idx = groupIdx(count);
    for (auto i = idx; i < m_freeGroupLists.size(); ++i) {
        for (Page* group = m_freeGroupLists.at(i); group != nullptr; group = group->next()) {
            if (group->groupSize() < count)
                continue;

            removeGroup(group);

            Page* allocatedGroup = nullptr;
            Page* remainingGroup = nullptr;
            std::tie(allocatedGroup, remainingGroup) = splitGroup(group, count);

            if (remainingGroup != nullptr)
                addGroup(remainingGroup);

            return allocatedGroup;
        }
    }

    return nullptr;
}

void PageAllocator::release(Page* pages)
{
    if (pages == nullptr)
        return;

    // clang-format off
    Page* joinedGroup = pages;

    // Try joining with pages above the released group.
    do {
        Page* lastAbove = joinedGroup->prevSibling();
        if (!isValidPage(lastAbove))
            break;

        if (getRegion(joinedGroup->address()) != getRegion(lastAbove->address()))
            break;

        if (lastAbove->isUsed())
            break;

        Page* firstAbove = lastAbove - lastAbove->groupSize() + 1;
        removeGroup(firstAbove);
        joinedGroup = joinGroup(firstAbove, joinedGroup);
    }
    while (true);

    // Try joining with pages below the released group.
    do {
        Page* lastJoined = joinedGroup + joinedGroup->groupSize() - 1;
        Page* firstBelow = lastJoined->nextSibling();
        if (!isValidPage(firstBelow))
            break;

        if (getRegion(lastJoined->address()) != getRegion(firstBelow->address()))
            break;

        if (firstBelow->isUsed())
            break;

        removeGroup(firstBelow);
        joinedGroup = joinGroup(joinedGroup, firstBelow);
    }
    while (true);
    // clang-format on

    addGroup(joinedGroup);
}

Page* PageAllocator::getPage(std::uintptr_t addr)
{
    auto alignedAddr = addr & ~(m_pageSize - 1);

    RegionInfo* pageRegion = getRegion(alignedAddr);
    if (pageRegion == nullptr)
        return nullptr;

    Page* result = nullptr;
    for (auto* page = pageRegion->firstPage; page <= pageRegion->lastPage; page = page->nextSibling()) {
        if (page->address() == alignedAddr) {
            result = page;
            break;
        }
    }

    return result;
}

PageAllocator::Stats PageAllocator::getStats()
{
    auto start = std::begin(m_regionsInfo);
    auto end = std::begin(m_regionsInfo) + m_validRegionsCount;

    Stats stats{};
    stats.totalMemorySize = std::accumulate(start, end, 0U, [](const size_t& sum, const RegionInfo& region) {
        return sum + region.size;
    });
    stats.effectiveMemorySize = std::accumulate(start, end, 0U, [](const size_t& sum, const RegionInfo& region) {
        return sum + region.alignedSize;
    });
    stats.userMemorySize = stats.effectiveMemorySize - (m_pageSize * m_descPagesCount);
    stats.freeMemorySize = m_freePagesCount * m_pageSize;
    stats.pageSize = m_pageSize;
    stats.totalPagesCount = m_pagesCount;
    stats.reservedPagesCount = m_descPagesCount;
    stats.freePagesCount = m_freePagesCount;

    return stats;
}

std::size_t PageAllocator::countPages()
{
    std::size_t pagesCount = 0;
    for (auto& region : m_regionsInfo)
        pagesCount += region.pageCount;

    return pagesCount;
}

std::size_t PageAllocator::chooseDescRegion()
{
    std::size_t descAreaSize = m_pagesCount * sizeof(Page);

    std::size_t selectedIdx = 0;
    for (std::size_t i = 0; i < m_validRegionsCount; ++i) {
        if (m_regionsInfo.at(i).alignedSize < descAreaSize)
            continue;

        if (m_regionsInfo.at(i).alignedSize < m_regionsInfo.at(selectedIdx).alignedSize)
            selectedIdx = i;
    }

    return selectedIdx;
}

std::size_t PageAllocator::reserveDescPages()
{
    std::size_t reservedCount = 0;
    auto& descRegion = m_regionsInfo.at(m_descRegionIdx);
    for (auto* page = descRegion.firstPage; page <= m_pagesTail; page = page->nextSibling()) {
        if (page->address() >= reinterpret_cast<uintptr_t>(m_pagesTail->nextSibling()))
            break;

        page->setUsed(true);
        ++reservedCount;

        if (page == descRegion.lastPage)
            break;
    }

    return reservedCount;
}

bool PageAllocator::isValidPage(Page* page)
{
    return (page >= m_pagesHead && page <= m_pagesTail);
}

RegionInfo* PageAllocator::getRegion(std::uintptr_t addr)
{
    auto alignedAddr = addr & ~(m_pageSize - 1);

    for (std::size_t i = 0; i < m_validRegionsCount; ++i) {
        auto& region = m_regionsInfo.at(i);

        if (region.alignedStart <= alignedAddr && region.alignedEnd >= alignedAddr)
            return &region;
    }

    return nullptr;
}

void PageAllocator::addGroup(Page* group)
{
    assert(group);

    std::size_t idx = groupIdx(group->groupSize());
    group->addToList(&m_freeGroupLists.at(idx));
    m_freePagesCount += group->groupSize();

    for (std::size_t i = 0; i < group->groupSize(); ++i) {
        auto* page = group + i;
        page->setUsed(false);
    }
}

void PageAllocator::removeGroup(Page* group)
{
    assert(group);

    std::size_t idx = groupIdx(group->groupSize());
    group->removeFromList(&m_freeGroupLists.at(idx));
    m_freePagesCount -= group->groupSize();

    for (std::size_t i = 0; i < group->groupSize(); ++i) {
        auto* page = group + i;
        page->setUsed(true);
    }
}

} // namespace memory
