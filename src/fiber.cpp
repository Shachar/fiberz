#include <fiberz/internal/fiber.h>

#include "utils.h"

#include <fiberz/reactor.h>
#include <fiberz/internal/fiber_parameters.h>

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
    if( next.getState() != State::Starting ) {
        ASSERT( next.getState() == State::Ready, "Fiber entering execution isn't READY" );
        next.setState( State::Unscheduled );
    }

    _context.switchTo(next._context);

    // Post switch callbacks
    if( _parameters && getState() != State::Starting ) {
        std::unique_ptr<ParametersBase> hook = std::move(_parameters);
        hook->invoke();
    }
}

void Fiber::start( std::unique_ptr<ParametersBase> params ) {
    ASSERT( _state == State::Free, "Trying to start fiber that is not FREE" );
    ASSERT( !_parameters, "Trying to set start parameters that are already set" );

    _generation++;

    setState( Fiber::State::Starting );
    _parameters = std::move( params );
}

void Fiber::main() {
    while(true) {
        ASSERT( !!_parameters, "Fiber running with no code to run" );
        ASSERT( getState() == State::Starting, "Starting fiber is not STARTING" );
        setState( State::Unscheduled );

        try {
            std::unique_ptr<ParametersBase> fiber_code = std::move(_parameters);
            fiber_code->invoke();
        } catch(std::exception &ex) {
            std::cerr<<"Fiber "<<getHandle()<<" terminated with unhandled exception "<<ex.what()<<"\n";
        } catch(FiberKilled &ex) {
            std::cout<<"Fiber "<<getHandle()<<" killed\n";
        }

        reactor().fiberDone( *this );
        reactor().switchToNext();
    }
}

} // namespace Fiberz::Internal
