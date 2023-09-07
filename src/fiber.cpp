#include "fiber.h"

namespace Fiberz::Internal {

Fiber::Fiber(void *stack_top, Idx idx) :
    _fiber_idx(idx),
    _generation(0)
{
    if( idx ) {
        _context.setup(stack_top, this, main_trampoline);
    }
}

void Fiber::main_trampoline(Fiber *_this) {
    _this->main();
}

void Fiber::main() {

    while(true) {
    }
}

} // namespace Fiberz::Internal
