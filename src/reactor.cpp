#include <fiberz/reactor.h>

#include <fiberz/exceptions.h>

#include <fiberz/internal/fiber_parameters.h>

#include "utils.h"

#include <sys/mman.h>
#include <unistd.h>

namespace Fiberz {

using namespace Internal;

thread_local std::optional<Reactor> the_reactor;

Reactor::Reactor(StartupParams startup_params) :
    _startup_params(startup_params),
    _stacks(
            nullptr,
            (startup_params.fiber_stack_pages + startup_params.fiber_stack_guard_pages) *
                startup_params.max_num_fibers * getpagesize(),
            PROT_READ|PROT_WRITE,
            MAP_STACK|MAP_SHARED
           ),
    _time_queue( now(), startup_params.num_time_queue_levels, startup_params.timer_resolution )
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

void Reactor::init(StartupParams params) {
    ASSERT( !the_reactor, "Tried to initialize an already initialized Fiberz reactor" );

    the_reactor.emplace( std::move(params) );
}

int Reactor::start() {
    // Main fiber main loop
    while( ! _ready_list.empty() ) {
        _fibers[0].setState( Fiber::State::Ready );     // Main fiber is always ready
        switchToNext();
    }

    std::cout<<"Normal reactor exit\n";

    return 0;
}

void Reactor::sleep(TimePoint wakeup) {
    auto thisFiber = currentFiberHandle();

    // By using the handle we guarantee that if the fiber is cancelled, the timer gets cancelled as well.
    auto timerHandle = registerCancellableTimer( wakeup, [this, thisFiber](TimePoint) { schedule(thisFiber); } );
    yield();
}

void Reactor::killFiber( FiberHandle handle ) {
    if( isValid(handle) )
        schedule( handle, true, []() { throw Internal::Fiber::FiberKilled(); } );
}

bool Reactor::isValid( FiberHandle handle ) const {
    if( !handle.isSet() )
        return false;

    return _fibers.at( handle.param1 )._generation == handle.param2 && _fibers.at(handle.param1).getState() != Fiber::State::Free;
}

// Private

void Reactor::switchToNext() {
    Fiber *next;
    if( _ready_list.empty() )
        next = &_fibers[0]; // Switch to main loop
    else {
        next = &_ready_list.front();
        _ready_list.pop_front();
    }

    switchTo( *next );
}

void Reactor::switchTo( Fiber &next ) {
    Fiber &current = currentFiber();

    if( &current == &next ) {
        ASSERT( next.getState()!=Fiber::State::Starting, "Starting fiber somehow scheduled itself" );

        next.setState( Fiber::State::Unscheduled );
        next.unlink();

        next.postSwitch();

        return;
    }

    _current_fiber = next.getIdx();

    current.switchTo( next );

    // After the switch
}

void Reactor::fiberDone( Fiber &fiber ) {
    ASSERT( fiber.getState() != Fiber::State::Free, "Terminating fiber already marked FREE" );
    fiber.setState( Fiber::State::Free );
    fiber.unlink();
    _free_list.push_front( fiber );
}

Fiber &Reactor::currentFiber() {
    return _fibers[_current_fiber.get()];
}

const Fiber &Reactor::currentFiber() const {
    return _fibers[_current_fiber.get()];
}

Internal::Fiber &Reactor::lookupFiber( FiberHandle handle ) {
    ASSERT( isValid( handle ), "Trying to dereference an invalid fiber handle" );

    return _fibers[ handle.param1 ];
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

void Reactor::schedule( FiberHandle handle, bool highPriority ) {
    if( !isValid(handle) )
        // XXX Log?
        return;

    schedule( lookupFiber( handle ), highPriority );
}

void Reactor::schedule( Fiber &fiber, bool highPriority ) {
    ASSERT( fiber.getState() != Fiber::State::Free, "Trying to schedule a free fiber" );

    if( highPriority ) {
        if( fiber.is_linked() )
            fiber.unlink();

        _ready_list.push_front(fiber);
    } else {
        auto fiberState = fiber.getState();
        if(
                fiberState == Fiber::State::Unscheduled ||
                (
                    fiberState == Fiber::State::Starting &&
                    !fiber.is_linked()
                )
          )
        {
            _ready_list.push_back(fiber);
        }
    }

    if( fiber.getState() == Fiber::State::Unscheduled )
        fiber.setState( Fiber::State::Ready );
}

void Reactor::schedule( FiberHandle handle, std::unique_ptr<Internal::ParametersBase> parameters, bool highPriority ) {
    schedule( lookupFiber( handle ), std::move( parameters ), highPriority );
}

void Reactor::schedule( Internal::Fiber &fiber, std::unique_ptr<Internal::ParametersBase> parameters, bool highPriority ) {
    ASSERT( fiber.getState() != Fiber::State::Free, "Trying to schedule a free fiber" );

    if( fiber._parameters ) {
        // XXX Log("Replacing previous callback with new one");
    }

    fiber._parameters = std::move(parameters);

    schedule( fiber, highPriority );
}


Reactor &reactor() {
    ASSERT( the_reactor.has_value(), "Trying to reference an uninitialized reactor" );

    return *the_reactor;
}

} // namespace Fiberz
