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

#include <cstddef>
#include <cstdint>
#include <optional>

namespace memory {

class Page;
struct Region;

/// @struct RegionInfo
/// Represents the meta data of the physical memory region.
struct RegionInfo {
    std::uintptr_t start;        ///< Physical address of the region start.
    std::uintptr_t end;          ///< Physical address of the region end.
    std::uintptr_t alignedStart; ///< Aligned physical address of the region start.
    std::uintptr_t alignedEnd;   ///< Aligned physical address of the region end.
    std::size_t pageCount;       ///< Number of pages, that this region contains.
    std::size_t size;            ///< Size of the region.
    std::size_t alignedSize;     ///< Size of the aligned part of the region.
    Page* firstPage;             ///< Pointer to the first page in the region.
    Page* lastPage;              ///< Pointer to the last page in the region.
};

/// Clears the contents of the region info.
/// @param[in,out]  regionInfo      Region info to be cleared.
void clearRegionInfo(RegionInfo& regionInfo);

/// Initializes the region info with the given region.
/// @param[in,out] regionInfo   Region info to be initialized.
/// @param[in] region           Region to be initialized from.
/// @param[in] pageSize         Size of the physical page.
/// @return Result of the initialization.
/// @retval true                Successfully initialized the region info.
/// @retval false               Failed to initialize the region info.
bool initRegionInfo(RegionInfo& regionInfo, const Region& region, std::size_t pageSize);

namespace detail {

/// Calculates the aligned physical address of the region start.
/// @param[in] region           Region from which address should be taken.
/// @param[in] pageSize         Size of the physical page.
/// @return Aligned physical address of the region start if exists, std::nullopt otherwise.
/// @retval std::uintptr_t      Aligned address of the region start.
/// @retval std::nullopt        Some error occurred.
std::optional<std::uintptr_t> alignedStart(const Region& region, std::size_t pageSize);

/// Calculates the aligned physical address of the region end.
/// @param[in] region           Region from which address should be taken.
/// @param[in] pageSize         Size of the physical page.
/// @return Aligned physical address of the region end if exists, std::nullopt otherwise.
/// @retval std::uintptr_t      Aligned address of the region end.
/// @retval std::nullopt        Some error occurred.
std::optional<std::uintptr_t> alignedEnd(const Region& region, std::size_t pageSize);

} // namespace detail
} // namespace memory
