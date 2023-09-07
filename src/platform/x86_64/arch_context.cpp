#include "../../context.h"

#include <stdint.h>

namespace Fiberz::Internal {

static void push(void *&stack_pointer, void *value) {
    auto sp = reinterpret_cast<void **>(stack_pointer);

    *(--sp) = value;

    stack_pointer = sp;
}

extern "C" void fiberz_fiber_startup_trampoline();

void Context::platform_setup(Fiber *fiber, void (*startup_function)(Fiber *)) {
    // Set up the initial stack
    void *base_pointer = _stack_pointer;

    // We only save the "callee saved" registers as defined by the X86_64 ABI
    push( _stack_pointer, nullptr );                                            // Fake return address of entrypoint
    push( _stack_pointer, reinterpret_cast<void *>(fiberz_fiber_startup_trampoline) );
                                                                                // RIP
    push( _stack_pointer, base_pointer );                                       // RBP
    push( _stack_pointer, nullptr );                                            // RBX
    push( _stack_pointer, nullptr );                                            // R12
    push( _stack_pointer, nullptr );                                            // R13
    push( _stack_pointer, reinterpret_cast<void *>(startup_function) );         // R14
    push( _stack_pointer, fiber );                                              // R15
}

} // namespace Fiberz::Internal
