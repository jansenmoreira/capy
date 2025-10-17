#include "capy.h"

// INTERNAL DEFINITIONS

static void task_set_stack(taskabi *abi, void *stack)
{
    abi->rsp = ((uintptr_t)stack);
}

static __attribute__((naked)) void task_switch_(Unused capy_task *next, Unused capy_task *current)
{
    __asm__(
        "mov %rsp, (%rsi)\n"
        "mov (%rdi), %rsp\n"
        "pop %r10\n"
        "mov %rbp, 8(%rsi)\n"
        "mov %rbx, 16(%rsi)\n"
        "mov %r12, 24(%rsi)\n"
        "mov %r13, 32(%rsi)\n"
        "mov %r14, 40(%rsi)\n"
        "mov %r15, 48(%rsi)\n"
        "mov 8(%rdi), %rbp\n"
        "mov 16(%rdi), %rbx\n"
        "mov 24(%rdi), %r12\n"
        "mov 32(%rdi), %r13\n"
        "mov 40(%rdi), %r14\n"
        "mov 48(%rdi), %r15\n"
        "jmp *%r10\n");
}
