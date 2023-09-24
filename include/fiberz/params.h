#pragma once

#include <stddef.h>

namespace Fiberz {

/// Parameters sent on startup to initialize the reactor
struct Params {
    /// Maximum number of concurrent fibers
    size_t max_num_fibers = 50;
    /// Number of memory pages each fiber's stack gets
    size_t fiber_stack_pages = 4;
    /// Number of guard pages between fibers' stacks. All non-zero numbers effectively cost the same.
    size_t fiber_stack_guard_pages = 2;
};

} // namespace Fiberz
