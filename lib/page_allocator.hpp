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

#pragma once

#include "region_info.hpp"
#include "utils.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>

namespace memory {

class Page;
struct Region;

/// @class PageAllocator
/// Represents an allocator of physical pages.
class PageAllocator {
public:
    /// @class Stats
    /// Represents the statistical data of the PageAllocator.
    struct Stats {
        std::size_t totalMemorySize;     ///< Total size of the memory passed during initialization.
        std::size_t effectiveMemorySize; ///< Effective size of the memory, that can be used by the PageAllocator.
        std::size_t userMemorySize;      ///< Total size of the memory available to the user.
        std::size_t freeMemorySize;      ///< Size of the remaining user memory.
        std::size_t pageSize;            ///< Size of the page used by the PageAllocator.
        std::size_t totalPagesCount;     ///< Total number of the pages known to the PageAllocator.
        std::size_t reservedPagesCount;  ///< Number of pages reserved for the PageAllocator.
        std::size_t freePagesCount;      ///< Current number of the free pages.
    };

    /// Default constructor.
    PageAllocator() noexcept;

    /// Initializes the PageAllocator with the given memory model.
    /// @param[in] regions          Array of memory regions to be used by PageAllocator. Last entry should be zeroed.
    /// @param[in] pageSize         Size of the page on the current platform.
    /// @return Result of the initialization.
    /// @retval true                PageAllocator has been initialized.
    /// @retval false               Some error occurred.
    [[nodiscard]] bool init(Region* regions, std::size_t pageSize);

    /// Clears the internal state of the PageAllocator.
    void clear();

    /// Allocates the given number of physical pages.
    /// @param[in] count            Number of pages to be allocated.
    /// @return Result of the allocation.
    /// @retval Page*               A set of allocated pages on success.
    /// @retval nullptr             Some error occurred.
    /// @note All allocated pages must be from the same region.
    [[nodiscard]] Page* allocate(std::size_t count);

    /// Releases the given set of pages.
    /// @param[in] pages            List of pages to be released.
    void release(Page* pages);

    /// Returns the Page, which contains the given address.
    /// @param[in] addr             Address for which Page should be found.
    /// @return Result of the check.
    /// @retval Page*               Pointer to Page containing given address if found.
    /// @retval nullptr             There is no page with the given address.
    Page* getPage(std::uintptr_t addr);

    /// Returns the current statistics of PageAllocator.
    /// @return PageAllocator statistics.
    Stats getStats();

private:
    /// Returns the total number of pages from all known regions.
    /// @return Number of all pages from all known regions.
    std::size_t countPages();

    /// Returns the index of the best region to store the page descriptors.
    /// @return Index of the region, where page descriptors will be stored.
    std::size_t chooseDescRegion();

    /// Reserves the necessary number of pages to store the page descriptors.
    /// @return Number of pages, that are used to store the page descriptors.
    std::size_t reserveDescPages();

    /// Checks if the given page is valid.
    /// @param[in] page             Page to be checked.
    /// @return Flag indicating if the given page is valid.
    /// @retval true                Page is valid.
    /// @retval false               Page is invalid.
    bool isValidPage(Page* page);

    /// Returns the RegionInfo, which contains the given address.
    /// @param[in] addr             Address for which RegionInfo should be found.
    /// @return Result of the search.
    /// @retval RegionInfo*         Pointer to RegionInfo containing given address if found.
    /// @retval nullptr             No region contains the given address.
    RegionInfo* getRegion(std::uintptr_t addr);

    /// Adds given group to the array of free groups.
    /// @param[in,out] group        Group to be added.
    void addGroup(Page* group);

    /// Removes given group from the array of free groups.
    /// @param[in,out] group        Group to be removed.
    void removeGroup(Page* group);

public:
    static constexpr int cMinPageSize = 128; ///< Minimal supported size of the page.

private:
    static constexpr int cMaxRegionsCount = 8; ///< Maximal supported number of memory regions.
    static constexpr int cMaxGroupIdx = 20;    ///< Maximal index of the group in the free array.

private:
    std::array<RegionInfo, cMaxRegionsCount> m_regionsInfo{}; ///< Array describing all known regions.
    std::size_t m_validRegionsCount{};                        ///< Number of used regions.
    std::size_t m_pageSize{};                                 ///< Size of the page used on this platform.
    std::size_t m_descRegionIdx{};                            ///< Index of the region used to store page descriptors.
    std::size_t m_descPagesCount{};                           ///< Number of pages used to store page descriptors.
    Page* m_pagesHead{};                                      ///< Head of the page descriptors list.
    Page* m_pagesTail{};                                      ///< Tail of the page descriptors list.
    std::array<Page*, cMaxGroupIdx> m_freeGroupLists{};       ///< Array of the groups with free pages.
    std::size_t m_pagesCount{};                               ///< Total number of pages known to the PageAllocator.
    std::size_t m_freePagesCount{};                           ///< Current number of free pages.
};

namespace detail {

/// Checks if the page size has a proper value.
/// @param[in] pageSize             Page size to be checked.
/// @return Flag indicating if the page size has a proper value.
/// @retval true                    Page size is valid.
/// @retval false                   Page size is invalid.
/// @note This function checks if pageSize has the minimal size and if is a power of 2.
inline bool isValidPageSize(std::size_t pageSize)
{
    return (pageSize >= PageAllocator::cMinPageSize && utils::isPowerOf2(pageSize));
}

} // namespace detail
} // namespace memory
