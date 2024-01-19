#pragma once

#include <fiberz/params.h>
#include <fiberz/unique_mmap.h>

#include <fiberz/internal/fiber.h>
#include <fiberz/internal/fiber_parameters.h>
#include <fiberz/internal/cascaded_time_queue.h>

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

    /// Timer resolution
    Duration timer_resolution = 1ms;
    /// Time queue levels
    size_t num_time_queue_levels = 5;
};

class Reactor {
    //boost::intrusive::list<Internal::Fiber, boost::intrusive::constant_time_size<false>>
    Internal::FiberList                         _free_list, _ready_list;
    StartupParams                               _startup_params;
    std::vector<Internal::Fiber>                _fibers;
    UniqueMmap                                  _stacks;
    Internal::Fiber::Idx                        _current_fiber = Internal::Fiber::Idx(0);
    Internal::CascadedTimeQueue                 _time_queue;
    bool                                        _exit_requested = false;

public:
    explicit Reactor( StartupParams startup_params );
    Reactor(const Reactor &that) = delete;
    Reactor &operator=(const Reactor &that) = delete;
    Reactor(const Reactor &&that) = delete;
    Reactor &operator=(const Reactor &&that) = delete;

    ~Reactor();

    static void init(StartupParams startupParams = StartupParams{});

    int start();
    void stop();

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

    void sleep() {
        switchToNext();
    }

    void sleep(TimePoint wakeup);

    void killFiber( FiberHandle handle );

    void yield() {
        schedule( currentFiber() );
        sleep();
    }

    void schedule( FiberHandle handle, bool highPriority = false );

    template<typename F, typename... Arguments>
            requires std::invocable<F, Arguments...>
    void schedule( FiberHandle handle, bool highPriority, F &&callable, Arguments&&... arguments ) {
        schedule(
                handle,
                std::make_unique< Internal::Parameters<F, Arguments...> >(
                    std::forward<F>(callable),
                    std::forward<Arguments...>(arguments)...
                    ),
                highPriority
                );

    }

    FiberHandle currentFiberHandle() const {
        return currentFiber().getHandle();
    }

    FiberId currentFiberId() const {
        return currentFiber().getHandle().getId();
    }

    bool isValid( FiberHandle handle ) const;

    using TimerCallback = Internal::CascadedTimeQueue::Callback;
    using TimerHandle = Internal::CascadedTimeQueue::TimerHandle;

    void registerTimer( TimePoint expiery, TimerCallback callback );
    [[nodiscard]] TimerHandle registerCancellableTimer( TimePoint expiery, TimerCallback callback ) {
        return _time_queue.insertEventWithHandle( expiery, std::move(callback) );
    }
    [[nodiscard]] TimerHandle registerRecurringTimer( Duration period, TimerCallback callback ) {
        return _time_queue.insertRecurringEvent( period, std::move(callback) );
    }

private:
    friend Internal::Fiber;

    void switchToNext();
    void switchTo( Internal::Fiber &fiber );

    void fiberDone( Internal::Fiber &fiber );

    Internal::Fiber &currentFiber();
    const Internal::Fiber &currentFiber() const;

    Internal::Fiber &lookupFiber( FiberHandle handle );

    FiberHandle createFiber( std::unique_ptr<Internal::ParametersBase> parameters );
    void schedule( Internal::Fiber &fiber, bool highPriority = false );
    void schedule( FiberHandle handle, std::unique_ptr<Internal::ParametersBase> parameters, bool highPriority = false );
    void schedule( Internal::Fiber &fiber, std::unique_ptr<Internal::ParametersBase> parameters, bool highPriority = false );
};

Reactor &reactor();

} // namespace Fiberz
