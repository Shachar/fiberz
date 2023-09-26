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

using ListMode = boost::intrusive::link_mode< boost::intrusive::auto_unlink >;
using FiberList = boost::intrusive::list<Fiber, boost::intrusive::constant_time_size<false>>;
/*
using ListMode = boost::intrusive::link_mode< boost::intrusive::safe_link >;
using FiberList = boost::intrusive::list<Fiber>;
*/

class Fiber : public boost::intrusive::list_base_hook< ListMode > {
public:
    using Idx = Typed<"FiberIdx", FiberId::UnderlyingType, 0, std::ios::hex>;

private:
    class FiberKilled {};       // Exception object used to kill a running fiber.

    // Members
    Context                             _context;
    Idx                                 _fiber_idx;
    unsigned short                      _generation = 0;

    std::unique_ptr<ParametersBase>     _parameters;

    enum class State { Free, Starting, Ready, Unscheduled }
                                        _state = State::Free;

public:

    explicit Fiber(void *stack_top, Idx idx);

    Fiber(const Fiber &that) = delete;
    Fiber &operator=(const Fiber &that) = delete;

    Fiber(Fiber &&that) = default;
    Fiber &operator=(Fiber &&that) = default;

    void switchTo(Fiber &next);
    void postSwitch();

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

    friend std::ostream & operator<<( std::ostream &out, Fiber::State state ) {
        switch( state ) {
#define CASE(s) case Fiber::State::s: return out<<#s
            CASE(Free);
            CASE(Starting);
            CASE(Ready);
            CASE(Unscheduled);
#undef CASE
        }

        return out<<"Fiber::State("<<static_cast<int>(state)<<")";
    }
};


} // namespace Fiberz::Internal
