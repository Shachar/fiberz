#include "fiber.h"

namespace Fiberz::Internal {

void main_trampoline(Fiber *_this) {
    _this->main();
}

Fiber::Fiber(void *stack_top, Idx idx) :
    _fiber_idx(idx),
    _generation(0)
{
    if( idx ) {
        std::cout<<"Constructing fiber "<<this<<" "<<idx<<" stack top "<<stack_top<<"\n";
        _context.setup(stack_top, this, main_trampoline);
    }
}

void Fiber::switchTo(Fiber &next) {
    _context.switchTo(next._context);
}

void Fiber::main() {

    std::cout<<"Running inside fiber "<<static_cast<void *>(this)<<" "<<_fiber_idx<<"\n";

    while(true) {
    }
}

} // namespace Fiberz::Internal
