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

#include "region.hpp"

#include <cstddef>
#include <cstdint>

namespace memory::allocator {

/// @struct Stats
/// Represents the statistical data of the allocator.
struct Stats {
    std::size_t totalMemorySize;     ///< Total size of the memory passed during initialization.
    std::size_t reservedMemorySize;  ///< Size of the memory reserved for the liballocator or ignored due to alignment.
    std::size_t userMemorySize;      ///< Size of the memory available to the user.
    std::size_t allocatedMemorySize; ///< Size of the memory allocated by the user.
    std::size_t freeMemorySize;      ///< Size of the free user memory.
};

/// Returns version of liballocator.
/// @return Version of liballocator.
const char* version();

/// Initializes liballocator with the given array of memory regions and page size.
/// @param[in] regions      Array of memory regions to be used by liballocator. Last entry should be zeroed.
/// @param[in] pageSize     Size of the page on the current platform.
/// @return Result of the initialization.
/// @retval true            Allocator has been initialized.
/// @retval false           Some error occurred.
[[nodiscard]] bool init(Region* regions, std::size_t pageSize);

/// Initializes liballocator with the given array of memory boundaries and page size.
/// @param[in] start        Start address of a memory region to be used by liballocator.
/// @param[in] end          End address of a memory region to be used by liballocator.
/// @param[in] pageSize     Size of the page on the current platform.
/// @return Result of the initialization.
/// @retval true            Allocator has been initialized.
/// @retval false           Some error occurred.
/// @note This overload is equivalent to the above version of init() with only one memory region entry.
[[nodiscard]] bool init(std::uintptr_t start, std::uintptr_t end, std::size_t pageSize);

/// Clears the internal state of liballocator.
void clear();

/// Allocates memory block with the given size.
/// @param[in] size         Demanded size of the allocated memory block.
/// @return Result of the allocation.
/// @retval void*           Allocated memory block on success.
/// @retval nullptr         Some error occurred.
[[nodiscard]] void* allocate(std::size_t size);

/// Releases the memory block pointed by given pointer.
/// @param[in] ptr          Pointer to the memory block, that should be released.
/// @note If the given pointer is nullptr, then function exists without an error.
void release(void* ptr);

/// Returns the current statistics of the allocator.
/// @return liballocator statistics.
Stats getStats();

} // namespace memory::allocator
