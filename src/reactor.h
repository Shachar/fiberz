#pragma once

#include "fiber.h"

#include <fiberz/fiberz.h>
#include <fiberz/unique_mmap.h>

#include <iostream>
#include <optional>

namespace Fiberz::Internal {

static inline void ASSERT( bool cond, const char *message ) {
    if( !cond ) {
        std::cerr<<"ASSERTION FAILED: "<<message<<"\n";

        abort();
    }
}

class Reactor {
    boost::intrusive::list<Fiber>       _free_list;
    boost::intrusive::list<Fiber>       _ready_list;
    Params                              _startup_params;
    std::vector<Fiber>                  _fibers;
    UniqueMmap                          _stacks;
    Fiber::Idx                          _current_fiber = Fiber::Idx(0);

public:
    explicit Reactor( Params startup_params );
    Reactor(const Reactor &that) = delete;
    Reactor &operator=(const Reactor &that) = delete;
    Reactor(const Reactor &&that) = delete;
    Reactor &operator=(const Reactor &&that) = delete;

    ~Reactor();

    int start();

    FiberHandle createFiber( std::unique_ptr<ParametersBase> parameters );

    void schedule( Fiber &fiber );
    void sleep();

private:
    friend Fiber;

    void switchToNext();
    void switchTo( Fiber &fiber );

    void fiberDone( Fiber &fiber );

    Fiber &currentFiber();
};

extern thread_local std::optional<Reactor> the_reactor;

} // namespace Fiberz::Internal
