#pragma once

#include <fiberz/typed.h>

namespace Fiberz {

using FiberId = Typed<"FiberId", uint16_t, 0xffff, std::ios::hex, 4>;

struct FiberHandle {
    uint16_t param1;
    uint16_t param2;

    FiberHandle() : param1(0), param2(0) {}
    FiberHandle(uint16_t p1, uint16_t p2) : param1(p1), param2(p2) {}

    FiberId getId() const;
};

} // namespace Fiberz
