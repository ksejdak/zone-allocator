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

#include "page.hpp"

#include <cassert>

namespace memory {

static_assert(Page::isNaturallyAligned(), "class Page is not naturally aligned");

void Page::init()
{
    initListNode();
    m_addr = 0;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    m_flags.value = 0;
}

void Page::setAddress(std::uintptr_t addr)
{
    m_addr = addr;
}

void Page::setGroupSize(std::size_t groupSize)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    m_flags.bits.groupSize = groupSize;
}

void Page::setUsed(bool value)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    m_flags.bits.used = value;
}

Page* Page::nextSibling()
{
    return (this + 1);
}

Page* Page::prevSibling()
{
    return (this - 1);
}

std::uintptr_t Page::address()
{
    return m_addr;
}

std::size_t Page::groupSize()
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    return m_flags.bits.groupSize;
}

bool Page::isUsed()
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    return m_flags.bits.used;
}

} // namespace memory
