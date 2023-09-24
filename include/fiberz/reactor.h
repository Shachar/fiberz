#pragma once

#include <fiberz/params.h>
#include <fiberz/unique_mmap.h>

#include <fiberz/internal/fiber.h>
#include <fiberz/internal/fiber_parameters.h>

#include <iostream>
#include <optional>

namespace Fiberz {

/// Parameters sent on startup to initialize the reactor
struct StartupParams {
    /// Maximum number of concurrent fibers
    size_t max_num_fibers = 50;
    /// Number of memory pages each fiber's stack gets
    size_t fiber_stack_pages = 4;
    /// Number of guard pages between fibers' stacks. All non-zero numbers effectively cost the same.
    size_t fiber_stack_guard_pages = 2;
};

class Reactor {
    boost::intrusive::list<Internal::Fiber>     _free_list;
    boost::intrusive::list<Internal::Fiber>     _ready_list;
    StartupParams                               _startup_params;
    std::vector<Internal::Fiber>                _fibers;
    UniqueMmap                                  _stacks;
    Internal::Fiber::Idx                        _current_fiber = Internal::Fiber::Idx(0);

public:
    explicit Reactor( StartupParams startup_params );
    Reactor(const Reactor &that) = delete;
    Reactor &operator=(const Reactor &that) = delete;
    Reactor(const Reactor &&that) = delete;
    Reactor &operator=(const Reactor &&that) = delete;

    ~Reactor();

    static void init(StartupParams startupParams = StartupParams{});

    int start();

    template<typename F, typename... Arguments>
            requires std::invocable<F, Arguments...>
    FiberHandle createFiber(F &&callable, Arguments&&... arguments) {
        return createFiber(
                std::make_unique< Internal::Parameters<F, Arguments...> >(
                    std::forward<F>(callable),
                    std::forward<Arguments...>(arguments)...
                    )
                );
    }

    void sleep();
    void yield() {
        schedule( currentFiber() );
        sleep();
    }

    FiberHandle currentFiberHandle() const {
        return currentFiber().getHandle();
    }

    FiberId currentFiberId() const {
        return currentFiber().getHandle().getId();
    }

private:
    friend Internal::Fiber;

    void switchToNext();
    void switchTo( Internal::Fiber &fiber );

    void fiberDone( Internal::Fiber &fiber );

    Internal::Fiber &currentFiber();
    const Internal::Fiber &currentFiber() const;

    FiberHandle createFiber( std::unique_ptr<Internal::ParametersBase> parameters );
    void schedule( Internal::Fiber &fiber );
};

Reactor &reactor();

} // namespace Fiberz
