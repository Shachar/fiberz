#pragma once

#include <chrono>

using namespace std::literals::chrono_literals;

namespace Fiberz {
    using Clock = std::chrono::steady_clock;
    using Duration = Clock::duration;

    class TimePoint : public Clock::time_point {
    public:
        using Clock::time_point::time_point;

        /* implicit */ TimePoint( Clock::time_point tp ) : Clock::time_point(tp) {}
        template< class Rep, class Period >
        /* implicit */ TimePoint( std::chrono::duration<Rep, Period> duration ) :
            Clock::time_point( Clock::now() + duration )
        {}
    };

    static inline TimePoint now() { return Clock::now(); }
} // namespace Fiberz
