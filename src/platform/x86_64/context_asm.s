.text

.global fiberz_fiber_startup_trampoline
.type fiberz_fiber_startup_trampoline, %function
fiberz_fiber_startup_trampoline:
    # First function argument is in RSI. For that very reason, however, it's not stored in the context.
    # So we move Fiber ptr from R15, which is stored, to RSI
    mov %r15, %rdi

    # Since we don't have a proper stack frame, do a jmp here, not a call
    jmp *%r14

.global fiberz_fiber_context_switch
.type fiberz_fiber_context_switch, %function
fiberz_fiber_context_switch:
    # First argument %rdi = pointer to where to save old stack pointer
    # Second argument %rsi = the new stack pointer

    # Save the exiting context to the stack
    push %rbp
    push %rbx
    push %r12
    push %r13
    push %r14
    push %r15
    movq %rsp, (%rdi)

    # Load new context
    mov %rsi, %rsp
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbx
    pop %rbp

    ret

# Tell the linker we don't need executable stack.
# In fact, our fibers' stack is not executable no matter what we say here, as the linker isn't the one allocating it.
.section .note.GNU-stack,"",@progbits
