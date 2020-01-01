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
#include "version.hpp"
#include "zone_allocator.hpp"

#include <allocator/allocator.hpp>

#include <array>

namespace {

memory::PageAllocator pageAllocator; // NOLINT(fuchsia-statically-constructed-objects)
memory::ZoneAllocator zoneAllocator; // NOLINT(fuchsia-statically-constructed-objects)

} // namespace

namespace memory::allocator {

const char* version()
{
    return cLiballocatorVersion;
}

bool init(Region* regions, std::size_t pageSize)
{
    clear();

    if (!pageAllocator.init(regions, pageSize))
        return false;

    return zoneAllocator.init(&pageAllocator, pageSize);
}

bool init(std::uintptr_t start, std::uintptr_t end, std::size_t pageSize)
{
    std::array<Region, 2> regions = {{{start, end - start}, {0, 0}}};

    return init(regions.data(), pageSize);
}

void clear()
{
    pageAllocator.clear();
    zoneAllocator.clear();
}

void* allocate(std::size_t size)
{
    return zoneAllocator.allocate(size);
}

void release(void* ptr)
{
    zoneAllocator.release(ptr);
}

Stats getStats()
{
    PageAllocator::Stats pageStats = pageAllocator.getStats();
    ZoneAllocator::Stats zoneStats = zoneAllocator.getStats();

    Stats stats{};
    stats.totalMemorySize = pageStats.totalMemorySize;
    // clang-format off
    stats.reservedMemorySize = pageStats.totalMemorySize - pageStats.effectiveMemorySize // Lost due to the alignment.
                               + pageStats.reservedPagesCount * pageStats.pageSize       // Reserved by the PageAllocator.
                               + zoneStats.reservedMemorySize;                           // Reserved by the ZoneAllocator.
    stats.userMemorySize = stats.totalMemorySize - stats.reservedMemorySize;
    stats.allocatedMemorySize = pageStats.userMemorySize - pageStats.freeMemorySize - zoneStats.usedMemorySize // Allocated from PageAllocator by user.
                                + zoneStats.allocatedMemorySize;                                               // Allocated from ZoneAllocator by user.
    stats.freeMemorySize = stats.userMemorySize - stats.allocatedMemorySize;
    // clang-format on

    return stats;
}

} // namespace memory::allocator
