#include "context.h"

extern "C" void fiberz_fiber_context_switch(void **old_sp, void *new_sp);

namespace Fiberz::Internal {

void Context::setup(void *stack_top, Fiber *fiber, void (*startup_function)(Fiber *)) {
    _stack_pointer = stack_top;

    platform_setup(fiber, startup_function);
}

void Context::switchTo(Context &next) {
    fiberz_fiber_context_switch(&_stack_pointer, next._stack_pointer);
}

} // namespace Fiberz::Internal
