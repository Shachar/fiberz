#pragma once

namespace Fiberz {

struct FiberHandle {
    uint16_t param1;
    uint16_t param2;

    FiberHandle() : param1(0), param2(0xffff) {}
    FiberHandle(uint16_t p1, uint16_t p2) : param1(p1), param2(p2) {}
};

} // namespace Fiberz
