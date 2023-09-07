.text

.global fiberz_fiber_startup_trampoline
.type fiberz_fiber_startup_trampoline, %function
fiberz_fiber_startup_trampoline:
    # First function argument is in RSI. For that very reason, however, it's not stored in the context.
    # So we move Fiber ptr from R15, which is stored, to RSI
    mov %RSI, %R15

    # Since we don't have a proper stack frame, do a jmp here, not a call
    jmp *%rdi

# Tell the linker we don't need executable stack.
# In fact, our fibers' stack is not executable no matter what we say here, as the linker isn't the one allocating it.
.section .note.GNU-stack,"",@progbits
