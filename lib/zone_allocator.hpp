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

#pragma once

#include "utils.hpp"
#include "zone.hpp"

#include <array>
#include <cmath>
#include <cstddef>

namespace memory {

class PageAllocator;

namespace detail {

std::size_t zoneIdx(std::size_t chunkSize);

} // namespace detail

/// @class ZoneAllocator
/// Represents the ZoneAllocator.
class ZoneAllocator {
public:
    /// @struct Stats
    /// Represents the statistical data of the ZoneAllocator.
    struct Stats {
        std::size_t usedMemorySize;      ///< Size of the memory that is under the control of the ZoneAllocator
        std::size_t reservedMemorySize;  ///< Size of the memory reserved for the ZoneAllocator within allocated zones.
        std::size_t freeMemorySize;      ///< Size of the free memory within allocated zones.
        std::size_t allocatedMemorySize; ///< Size of the memory allocated by the user within allocated zones.
    };

    /// Default constructor.
    ZoneAllocator() noexcept;

    /// Initializes the ZoneAllocator with the given PageAllocator and page size.
    /// @param[in] pageAllocator        PageAllocator to be used in ZoneAllocator.
    /// @param[in] pageSize             Size of the physical page.
    /// @return Result of the initialization.
    /// @retval true                    ZoneAllocator has been initialized.
    /// @retval false                   Some error occurred.
    [[nodiscard]] bool init(PageAllocator* pageAllocator, std::size_t pageSize);

    /// Clears the ZoneAllocator internal state.
    void clear();

    /// Allocates the memory chunk of at least given size.
    /// @param[in] size                 Size of the demanded memory chunk.
    /// @return Result of the allocation.
    /// @retval void*                   Pointer to the allocated memory chunk on success.
    /// @retval nullptr                 Some error occurred.
    [[nodiscard]] void* allocate(std::size_t size);

    /// Releases the given memory chunk.
    /// @param[in] ptr                  Pointer to the memory chunk to be released.
    /// @note This function accepts nullptr input.
    void release(void* ptr);

    /// Returns the current statistics of ZoneAllocator.
    /// @return ZoneAllocator statistics.
    Stats getStats();

private:
    /// Allocates memory chunk from the given zone.
    /// @param[in] zone                 Zone from which chunk should be allocated.
    /// @return Allocated memory chunk.
    /// @note Template parameter is used here to cast the returned value the given type.
    template <typename T>
    T* allocateChunk(Zone* zone)
    {
        std::size_t idx = detail::zoneIdx(zone->chunkSize());
        m_zones.at(idx).freeChunksCount--;
        return reinterpret_cast<T*>(zone->takeChunk());
    }

    /// Deallocates memory chunk to the given zone.
    /// @param[in] chunk                Chunk to be deallocated.
    /// @return Result of the chunk deallocation.
    /// @retval true                    Chunk has been deallocated.
    /// @retval false                   Chunk has not been deallocated.
    /// @note Template parameter is used here to accept any input without casting.
    template <typename T>
    bool deallocateChunk(T* chunk)
    {
        auto* zoneChunk = reinterpret_cast<Chunk*>(chunk);
        auto* zone = findZone(zoneChunk);
        if (!zone)
            return false;

        std::size_t idx = detail::zoneIdx(zone->chunkSize());
        m_zones.at(idx).freeChunksCount++;
        zone->giveChunk(zoneChunk);

        if (zone->chunksCount() == zone->freeChunksCount() && zone != &m_initialZone) {
            removeZone(zone);
            clearZone(zone);
            return deallocateChunk(zone);
        }

        return true;
    }

    /// Returns the Zone from the given array index, that has at least one free chunk.
    /// @param[in] idx                  Index from which Zone should be taken.
    /// @return Result of the search.
    /// @retval Zone*                   Pointer to the Zone on success.
    /// @retval nullptr                 No free zone was found.
    Zone* getFreeZone(std::size_t idx);

    /// Checks if there is the minimal required number of free chunks in the zone at given array index.
    /// @param[in] idx                  Index to be checked.
    /// @return Flag indicating if a new zone should be allocated.
    /// @retval true                    New Zone with the given index should be allocated.
    /// @retval false                   No need to allocate a new zone.
    bool shouldAllocateZone(std::size_t idx);

    /// Allocates new Zone with the chunks of given size.
    /// @param[in] chunkSize            Size of the chunks in the allocated zone.
    /// @return Result of the allocation.
    /// @retval Zone*                   Pointer to the allocated Zone on success.
    /// @retval nullptr                 Some error occurred.
    Zone* allocateZone(std::size_t chunkSize);

    /// Initializes given zone.
    /// @param[in,out] zone             Zone to be initialized.
    /// @param[in] chunkSize            Size of the chunks in this zone.
    /// @return Result of the initialization.
    /// @retval true                    Zone has been initialized.
    /// @retval false                   Some error occurred.
    bool initZone(Zone* zone, std::size_t chunkSize);

    /// Clears the given zone.
    /// @param[in,out] zone             Zone to be cleared.
    void clearZone(Zone* zone);

    /// Adds the given zone to the array of known zones.
    /// @param[in,out] zone             Zone to be added.
    void addZone(Zone* zone);

    /// Removes the given zone from the array of known zones.
    /// @param[in,out] zone             Zone to be removed.
    void removeZone(Zone* zone);

    /// Finds the Zone that given chunk belong to.
    /// @param[in,out] chunk            Chunk for which zone should be found.
    /// @return Result of the search.
    /// @retval Zone*                   Zone that given chunk belong to if found.
    /// @retval nullptr                 Zone has not been found.
    Zone* findZone(Chunk* chunk);

public:
    static constexpr std::size_t cMinimalAllocSize = 16; ///< Minimal size of chunk, that can be allocated.

private:
    static constexpr std::size_t cMaxZoneIdx = 8; ///< Maximal supported entries in the zone array.

private:
    /// @struct ZoneInfo
    /// Represents the meta-data of the zone.
    struct ZoneInfo {
        Zone* head{};                  ///< Head of the zones with the given index.
        std::size_t freeChunksCount{}; ///< Total number of free chunks in zones with the given index.
    };

    PageAllocator* m_pageAllocator{};            ///< PageAllocator to be used as the source of the new pages.
    std::size_t m_pageSize{};                    ///< Size of the page on this platform.
    std::size_t m_zoneDescChunkSize{};           ///< Size of the chunks that are used to store zone descriptors.
    std::size_t m_zoneDescIdx{};                 ///< Index of the zones, from which zone descriptors are allocated.
    Zone m_initialZone{};                        ///< Initial static zone.
    std::array<ZoneInfo, cMaxZoneIdx> m_zones{}; ///< Array of all zones known in the ZoneAllocator.
};

namespace detail {

/// Returns size rounded up to the closest chunk size.
/// @param[in] size                     Size to be rounded up.
/// @return Closest chunk size.
inline std::size_t chunkSize(std::size_t size)
{
    std::size_t chunkSize = std::max(size, ZoneAllocator::cMinimalAllocSize);
    return utils::roundPowerOf2(chunkSize);
}

/// Returns an index of the zone with the given chunk size.
/// @param[in] chunkSize                Chunk size to be used in calculations.
/// @return Index of the zone in the array of all known zones.
inline std::size_t zoneIdx(std::size_t chunkSize)
{
    return static_cast<std::size_t>(std::floor(std::log2(chunkSize)) - 4);
}

} // namespace detail
} // namespace memory
