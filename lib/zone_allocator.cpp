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

#include "zone_allocator.hpp"

#include "page.hpp"
#include "page_allocator.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

namespace memory {

ZoneAllocator::ZoneAllocator() noexcept
{
    clear();
}

bool ZoneAllocator::init(PageAllocator* pageAllocator, std::size_t pageSize)
{
    clear();

    m_pageAllocator = pageAllocator;
    m_pageSize = pageSize;
    m_zoneDescChunkSize = detail::chunkSize(sizeof(Zone));
    m_zoneDescIdx = detail::zoneIdx(m_zoneDescChunkSize);
    if (!initZone(&m_initialZone, m_zoneDescChunkSize))
        return false;

    addZone(&m_initialZone);
    return true;
}

void ZoneAllocator::clear()
{
    m_pageAllocator = nullptr;
    m_pageSize = 0;
    m_zoneDescChunkSize = 0;
    m_zoneDescIdx = 0;
    m_initialZone.clear();
    m_zones.fill({});
}

void* ZoneAllocator::allocate(std::size_t size)
{
    if (size == 0)
        return nullptr;

    if (size >= m_pageSize) {
        auto pageCount = static_cast<std::size_t>(std::ceil(double(size) / double(m_pageSize)));
        if (auto* page = m_pageAllocator->allocate(pageCount))
            return reinterpret_cast<void*>(page->address());

        return nullptr;
    }

    std::size_t allocSize = detail::chunkSize(size);
    std::size_t idx = detail::zoneIdx(allocSize);

    Zone* zone = shouldAllocateZone(idx) ? allocateZone(allocSize) : getFreeZone(idx);
    if (zone == nullptr)
        return nullptr;

    return allocateChunk<void>(zone);
}

void ZoneAllocator::release(void* ptr)
{
    if (ptr == nullptr)
        return;

    if (deallocateChunk(ptr))
        return;

    if (auto* pages = m_pageAllocator->getPage(std::uintptr_t(ptr)))
        m_pageAllocator->release(pages);
}

ZoneAllocator::Stats ZoneAllocator::getStats()
{
    auto start = std::begin(m_zones);
    auto end = std::end(m_zones);

    // clang-format off
    std::size_t usedZonesCount = std::accumulate(start, end, 0U, [](const size_t& sum, const ZoneInfo& zoneInfo) {
        std::size_t count = 0;
        for (auto* zone = zoneInfo.head; zone != nullptr; zone = zone->next(), ++count);
        return sum + count;
    });
    // clang-format on

    Stats stats{};
    stats.usedMemorySize = usedZonesCount * m_pageSize;
    stats.reservedMemorySize = (usedZonesCount > 0) ? (usedZonesCount - 1) * m_zoneDescChunkSize : 0;
    stats.freeMemorySize = std::accumulate(start, end, 0U, [](const size_t& sum, const ZoneInfo& zoneInfo) {
        if (zoneInfo.head == nullptr)
            return sum;

        return sum + (zoneInfo.head->chunkSize() * zoneInfo.freeChunksCount);
    });
    stats.allocatedMemorySize = stats.usedMemorySize - stats.reservedMemorySize - stats.freeMemorySize;

    return stats;
}

Zone* ZoneAllocator::getFreeZone(std::size_t idx)
{
    Zone* zone = nullptr;
    for (zone = m_zones.at(idx).head; zone != nullptr; zone = zone->next()) {
        if (zone->freeChunksCount() > 0)
            break;
    }

    return zone;
}

bool ZoneAllocator::shouldAllocateZone(std::size_t idx)
{
    std::size_t triggerCount = (idx == m_zoneDescIdx) ? 1 : 0;
    return (m_zones.at(idx).freeChunksCount == triggerCount);
}

Zone* ZoneAllocator::allocateZone(std::size_t chunkSize)
{
    if (chunkSize != m_zoneDescChunkSize && shouldAllocateZone(m_zoneDescIdx)) {
        if (allocateZone(m_zoneDescChunkSize) == nullptr)
            return nullptr;
    }

    auto* zone = getFreeZone(m_zoneDescIdx);
    assert(zone);
    auto* newZone = allocateChunk<Zone>(zone);
    assert(newZone);

    if (!initZone(newZone, chunkSize)) {
        deallocateChunk(newZone);
        return nullptr;
    }

    addZone(newZone);
    return newZone;
}

bool ZoneAllocator::initZone(Zone* zone, std::size_t chunkSize)
{
    assert(zone);

    if (auto* page = m_pageAllocator->allocate(1)) {
        zone->init(page, m_pageSize, chunkSize);
        return true;
    }

    return false;
}

void ZoneAllocator::clearZone(Zone* zone)
{
    assert(zone);

    m_pageAllocator->release(zone->page());
    zone->clear();
}

void ZoneAllocator::addZone(Zone* zone)
{
    assert(zone);

    auto idx = detail::zoneIdx(zone->chunkSize());
    zone->addToList(&m_zones.at(idx).head);
    m_zones.at(idx).freeChunksCount += zone->freeChunksCount();
}

void ZoneAllocator::removeZone(Zone* zone)
{
    assert(zone);
    assert(zone != &m_initialZone);

    auto idx = detail::zoneIdx(zone->chunkSize());
    zone->removeFromList(&m_zones.at(idx).head);
    m_zones.at(idx).freeChunksCount -= zone->freeChunksCount();
}

Zone* ZoneAllocator::findZone(Chunk* chunk)
{
    assert(chunk);

    auto chunkAddr = reinterpret_cast<std::uintptr_t>(chunk);
    auto pageAddr = chunkAddr & ~(m_pageSize - 1);

    for (auto& zoneInfo : m_zones) {
        for (auto* zone = zoneInfo.head; zone != nullptr; zone = zone->next()) {
            if (zone->page()->address() == pageAddr && zone->isValidChunk(chunk))
                return zone;
        }
    }

    return nullptr;
}

} // namespace memory
