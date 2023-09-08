#include "reactor.h"

#include <fiberz/exceptions.h>

#include <sys/mman.h>
#include <unistd.h>

namespace Fiberz::Internal {

thread_local std::optional<Reactor> the_reactor;

Reactor::Reactor(Params startup_params) :
    _startup_params(startup_params),
    _stacks(
            nullptr,
            (startup_params.fiber_stack_pages + startup_params.fiber_stack_guard_pages) *
                startup_params.max_num_fibers * getpagesize(),
            PROT_READ|PROT_WRITE,
            MAP_STACK|MAP_SHARED
           )
{
    _fibers.reserve(startup_params.max_num_fibers);

    // Create the guard pages
    std::byte *stack = _stacks.get();
    const size_t page_size = getpagesize();
    for( Fiber::Idx::UnderlyingType i=0; i<_startup_params.max_num_fibers; ++i ) {
        mprotect( stack, page_size * _startup_params.fiber_stack_guard_pages, PROT_NONE );

        stack += page_size * (_startup_params.fiber_stack_guard_pages + _startup_params.fiber_stack_pages);
        _fibers.emplace_back(stack, Fiber::Idx(i));

        if( i>0 )
            _free_list.push_back( _fibers[i] );
    }
}

Reactor::~Reactor() {
    _ready_list.clear();
    _free_list.clear();
}

int Reactor::start() {
    while( ! _ready_list.empty() ) {
        switchToNext();
    }

    return 0;
}

FiberHandle Reactor::createFiber( std::unique_ptr<ParametersBase> parameters ) {
    if( _free_list.empty() )
        throw NoFreeFibers();

    Fiber &free_fiber = _free_list.front();
    _free_list.pop_front();

    free_fiber.start( std::move(parameters) );

    schedule(free_fiber);

    return free_fiber.getHandle();
}

void Reactor::schedule( Fiber &fiber ) {
    ASSERT( fiber.getState() != Fiber::State::Free, "Trying to schedule a free fiber" );
    _ready_list.push_back(fiber);
    fiber.setState( Fiber::State::Ready );
}

void Reactor::sleep() {
    Fiber &current = currentFiber();
    if( current.getState() != Fiber::State::Free )
        current.setState( Fiber::State::Waiting );

    switchToNext();
}

void Reactor::switchToNext() {
    Fiber *next;
    if( _ready_list.empty() )
        next = &_fibers[0]; // Switch to main loop
    else {
        next = &_ready_list.front();
        _ready_list.pop_front();
        next->setState( Fiber::State::Running );
    }

    switchTo( *next );
}

void Reactor::switchTo( Fiber &next ) {
    Fiber &current = currentFiber();

    if( &current == &next ) {
        ASSERT( current.getState()==Fiber::State::Running, "Running fiber isn't marked running" );

        // XXX Perform context switch tests
        return;
    }

    if( current.getState()==Fiber::State::Running )
        current.setState( Fiber::State::Waiting );

    _current_fiber = next.getIdx();

    current.switchTo( next );
}

void Reactor::fiberDone( Fiber &fiber ) {
    ASSERT( fiber.getState() == Fiber::State::Running, "Fiber not running is marked as done" );
    fiber.setState( Fiber::State::Free );
    _free_list.push_front( fiber );
}

Fiber &Reactor::currentFiber() {
    return _fibers[_current_fiber.get()];
}

const Fiber &Reactor::currentFiber() const {
    return _fibers[_current_fiber.get()];
}

} // namespace Fiberz
