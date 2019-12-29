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

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <memory>

namespace test {

inline std::unique_ptr<std::byte, decltype(&std::free)> alignedAlloc(std::size_t alignment, std::size_t size)
{
    void* ptr = nullptr;

    // TODO(kuba): This is a workaround for bug in GCC lacking std::aligned_alloc().
    // return std::unique_ptr<std::byte, decltype(&std::free)>(std::aligned_alloc(alignment, size));
    auto result = posix_memalign(&ptr, alignment, size);
    (void) result;

    return std::unique_ptr<std::byte, decltype(&std::free)>(reinterpret_cast<std::byte*>(ptr), &std::free);
}

inline std::chrono::time_point<std::chrono::high_resolution_clock> currentTime()
{
    return std::chrono::high_resolution_clock::now();
}

template <typename T>
inline bool timeElapsed(std::chrono::time_point<std::chrono::high_resolution_clock>& start, const T& duration)
{
    auto elapsedSeconds = currentTime() - start;
    return (elapsedSeconds >= duration);
}

inline double toMicroseconds(const std::chrono::duration<double>& duration)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

} // namespace test
