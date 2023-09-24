#pragma once

#include <fiberz/fiber_handle.h>

#include <fiberz/internal/context.h>

#include <boost/intrusive/list.hpp>

#include <memory>

namespace Fiberz {
class Reactor;
}

namespace Fiberz::Internal {

class Fiber;
class ParametersBase;

extern "C" void main_trampoline(Fiber *_this);

class Fiber : public boost::intrusive::list_base_hook<> {
public:
    using Idx = Typed<"FiberIdx", FiberId::UnderlyingType, 0, std::ios::hex>;

private:
    // Members
    Context                             _context;
    Idx                                 _fiber_idx;
    unsigned short                      _generation = 0;

    std::unique_ptr<ParametersBase>     _parameters;

    enum class State { Free, Ready, Running, Waiting }
                                        _state = State::Free;

public:

    explicit Fiber(void *stack_top, Idx idx);

    Fiber(const Fiber &that) = delete;
    Fiber &operator=(const Fiber &that) = delete;

    Fiber(Fiber &&that) = default;
    Fiber &operator=(Fiber &&that) = default;

    void switchTo(Fiber &next);

    void start( std::unique_ptr<ParametersBase> params );

    FiberHandle getHandle() const {
        return FiberHandle( _fiber_idx.get(), _generation );
    }

    Idx getIdx() const {
        return _fiber_idx;
    }

private:
    friend void main_trampoline(Fiber *fiber);
    friend Reactor;

    void main();

    State getState() const {
        return _state;
    }
    void setState(State state) {
        _state = state;
    }
};

} // namespace Fiberz::Internal
