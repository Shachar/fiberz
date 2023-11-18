#pragma once

#include <chrono>

using namespace std::literals::chrono_literals;

namespace Fiberz {
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;

    static inline TimePoint now() { return Clock::now(); }
} // namespace Fiberz
