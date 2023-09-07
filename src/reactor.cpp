#include "reactor.h"

#include <sys/mman.h>
#include <unistd.h>

namespace Fiberz::Internal {

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
    }

    for( Fiber::Idx::UnderlyingType i=_startup_params.max_num_fibers-1; i>0; --i ) {
        _free_list.push_back( _fibers[i] );
    }
}

} // namespace Fiberz
