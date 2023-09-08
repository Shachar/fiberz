#include "fiber.h"

#include "reactor.h"

namespace Fiberz {

FiberId FiberHandle::getId() const {
    static constexpr uint32_t COEFF = 0xb0f2df91;

    return FiberId( param2 * COEFF + param1 );
}

}

namespace Fiberz::Internal {

void main_trampoline(Fiber *_this) {
    _this->main();
}

Fiber::Fiber(void *stack_top, Idx idx) :
    _fiber_idx(idx),
    _generation(0)
{
    if( idx ) {
        _context.setup(stack_top, this, main_trampoline);
    }
}

void Fiber::switchTo(Fiber &next) {
    _context.switchTo(next._context);
}

void Fiber::start( std::unique_ptr<ParametersBase> params ) {
    ASSERT( _state == State::Free, "Trying to start fiber that is already running" );
    ASSERT( !_parameters, "Trying to set start parameters that are already set" );

    _generation++;

    _state = State::Waiting;
    _parameters = std::move( params );
}

void Fiber::main() {
    while(true) {
        ASSERT( !!_parameters, "Fiber running with no code to run" );
        ASSERT( getState() == State::Running, "Running fiber is not marked running" );

        std::unique_ptr<ParametersBase> fiber_code = std::move(_parameters);
        fiber_code->invoke();

        the_reactor->fiberDone( *this );
        the_reactor->sleep();
    }
}

} // namespace Fiberz::Internal
