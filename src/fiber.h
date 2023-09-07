#pragma once

#include <fiberz/fiberz.h>

#include "context.h"

namespace Fiberz::Internal {

class Fiber {
public:
    using Idx = Typed<"FiberIdx", FiberId::UnderlyingType, 0, std::ios::hex>;

private:
    // Members
    Context                     _context;
    Idx                         _fiber_idx;
    unsigned short              _generation = 0;

public:

    explicit Fiber(void *stack_top, Idx idx);

    Fiber(const Fiber &that) = delete;
    Fiber &operator=(const Fiber &that) = delete;

    Fiber(Fiber &&that) = default;
    Fiber &operator=(Fiber &&that) = default;

private:
    static void main_trampoline(Fiber *_this);
    void main();
};

} // namespace Fiberz::Internal
