#pragma once

#include "fiber.h"

#include <fiberz/fiberz.h>
#include <fiberz/unique_mmap.h>

#include <iostream>

namespace Fiberz::Internal {

static inline void ASSERT( bool cond, const char *message ) {
    if( !cond ) {
        std::cerr<<"ASSERTION FAILED: "<<message<<"\n";

        abort();
    }
}

class Reactor {
    boost::intrusive::list<Fiber>       _free_list;
    Params                              _startup_params;
    std::vector<Fiber>                  _fibers;
    UniqueMmap                          _stacks;

public:
    explicit Reactor( Params startup_params );
    Reactor(const Reactor &that) = delete;
    Reactor &operator=(const Reactor &that) = delete;
    Reactor(const Reactor &&that) = delete;
    Reactor &operator=(const Reactor &&that) = delete;

};

} // namespace Fiberz::Internal
