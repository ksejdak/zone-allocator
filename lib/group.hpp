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
#include <tuple>

namespace memory {

class Page;

/// Calculates index in the groups array, for which group with the given page count should be stored.
/// @param[in] pageCount        Number of pages, for which index should be calculated.
/// @return Index in the groups array.
std::size_t groupIdx(std::size_t pageCount);

/// Initializes the given group.
/// @param[in,out] group        Group to be initialized.
/// @param[in] groupSize        Size of the initialized group.
void initGroup(Page* group, std::size_t groupSize);

/// Clears the given group.
/// @param[in,out] group        Group to be cleared.
void clearGroup(Page* group);

/// Splits the given group into one of the given size and second with the remaining size.
/// @param[in] group            Group to be splitted.
/// @param[in] size             Target size of the first group.
/// @returns Tuple with group of demanded size and with the group of the remaining size.
/// @note If the given group has already the correct size, then second pointer in the tuple is nullptr.
std::tuple<Page*, Page*> splitGroup(Page* group, std::size_t size);

/// Joins two given groups into one.
/// @param[in] firstGroup       First group to be joined.
/// @param[in] secondGroup      Second group to be joined.
/// @return Group that is a sum of the two given groups.
Page* joinGroup(Page* firstGroup, Page* secondGroup);

} // namespace memory
