#pragma once

#include <fiberz/typed.h>

namespace Fiberz {

using FiberId = Typed<"FiberId", uint16_t, 0xffff, std::ios::hex, 4>;

class Reactor {
public:
    struct Params {
    };

    static void init(Params params = Params());
};

} // namespace Fiberz
