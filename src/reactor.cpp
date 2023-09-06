#include "reactor.h"

#include <sys/mman.h>
#include <unistd.h>

namespace Fiberz::Internal {

Reactor::Reactor(Params startup_params) :
    _startup_params(startup_params),
    _fibers(startup_params.max_num_fibers),
    _stacks(
            nullptr,
            (startup_params.fiber_stack_pages + startup_params.fiber_stack_guard_pages) *
                startup_params.max_num_fibers * getpagesize(),
            PROT_READ|PROT_WRITE,
            MAP_STACK|MAP_SHARED
           )
{
    // Create the guard pages
    std::byte *stack = _stacks.get();
    const size_t page_size = getpagesize();
    for( unsigned i=0; i<_startup_params.max_num_fibers; ++i ) {
        mprotect( stack, page_size * _startup_params.fiber_stack_guard_pages, PROT_NONE );

        stack += page_size * (_startup_params.fiber_stack_guard_pages + _startup_params.fiber_stack_pages);
    }
}

} // namespace Fiberz
