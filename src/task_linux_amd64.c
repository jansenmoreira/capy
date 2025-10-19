/*
  libco v20 (2019-10-16)
  author: byuu
  license: ISC
  ISC License (ISC)

  Copyright byuu and the higan team

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
  REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
  AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
  INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
  LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
  OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
  PERFORMANCE OF THIS SOFTWARE.
*/

#include "capy.h"

typedef struct capy_task_ctx
{
    uint64_t rsp;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
} capy_task_ctx;

capy_task_ctx *capy_task_ctx_init(capy_arena *arena, void *stack_, void (*entrypoint)(void))
{
    capy_task_ctx *ctx = Make(arena, capy_task_ctx, 1);

    if (ctx == NULL)
    {
        return NULL;
    }

    if (stack_ != NULL)
    {
        uint64_t *stack = stack_;

        *(--stack) = 0;
        *(--stack) = Cast(uint64_t, entrypoint);

        ctx->rsp = Cast(uint64_t, stack);
    }

    return ctx;
}

__attribute__((naked)) void capy_task_switch_(Unused capy_task_ctx *next, Unused capy_task_ctx *current)
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
