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

#include <capy/macros.h>

#if defined(__GNUC__) && defined(__amd64__)  // GCC/Clang AMD64
typedef struct capy_co
{
    uintptr_t rsp;
    uintptr_t rbp;
    uintptr_t rbx;
    uintptr_t r12;
    uintptr_t r13;
    uintptr_t r14;
    uintptr_t r15;
} capy_co;

static __attribute__((naked)) void co_swap(Unused capy_co *next, Unused capy_co *current)
{
    __asm__(
        "mov [rsi], rsp\n"
        "mov rsp, [rdi]\n"
        "pop rax\n"
        "mov [rsi+ 8], rbp\n"
        "mov [rsi+16], rbx\n"
        "mov [rsi+24], r12\n"
        "mov [rsi+32], r13\n"
        "mov [rsi+40], r14\n"
        "mov [rsi+48], r15\n"
        "mov rbp, [rdi+ 8]\n"
        "mov rbx, [rdi+16]\n"
        "mov r12, [rdi+24]\n"
        "mov r13, [rdi+32]\n"
        "mov r14, [rdi+40]\n"
        "mov r15, [rdi+48]\n"
        "jmp rax");
}
#endif

static thread_local capy_co co_main;
static thread_local capy_co *co_active = NULL;

static void co_crash(void)
{
    abort();
}

static void co_init(void)
{
    if (co_active == NULL)
    {
        co_main = (capy_co){0};
        co_active = &co_main;
    }
}

capy_co *capy_co_active(void)
{
    co_init();
    return co_active;
}

capy_co *capy_co_init(capy_arena *arena, size_t size, void (*entrypoint)(void))
{
    co_init();

    uintptr_t *stack = capy_arena_create_stack(arena, size);

    if (stack == NULL)
    {
        return NULL;
    }

    *(--stack) = Cast(uintptr_t, co_crash);
    *(--stack) = Cast(uintptr_t, entrypoint);

    capy_co *co = Make(arena, capy_co, 1);

    if (co == NULL)
    {
        return NULL;
    }

    co->rsp = Cast(uintptr_t, stack);

    return co;
}

void capy_co_switch(capy_co *handle)
{
    register capy_co *co_previous_handle = co_active;
    co_swap(co_active = handle, co_previous_handle);
}
