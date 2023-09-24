#pragma once

namespace Fiberz::Internal {

class Fiber;

class Context {
    void *_stack_pointer = nullptr;
public:

    explicit Context() = default;
    Context(const Context &that) = delete;
    Context &operator=(const Context &that) = delete;
    Context(Context &&that) = default;
    Context &operator=(Context &&that) = default;

    void setup(void *stack_top, Fiber *fiber, void (*startup_function)(Fiber *));

    void switchTo(Context &next);
private:
    void platform_setup(Fiber *fiber, void (*startup_function)(Fiber *));
    void fiber_startup();
};

} // namespace Fiberz::Internal
