#include "context.h"

namespace Fiberz::Internal {

void Context::setup(void *stack_top, Fiber *fiber, void (*startup_function)(Fiber *)) {
    _stack_pointer = stack_top;

    platform_setup(fiber, startup_function);
}

} // namespace Fiberz::Internal
