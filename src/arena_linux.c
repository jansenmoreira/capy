#include <capy/capy.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

struct capy_arena
{
    size_t size;
    size_t capacity;
    size_t limit;
    size_t page_size;
};

capy_arena *capy_arena_init(size_t limit)
{
    capy_arena *arena = mmap(NULL, limit, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (arena == MAP_FAILED)
    {
        capy_log_errno(errno, "mmap failed");
        abort();
    }

    size_t page_size = (size_t)(sysconf(_SC_PAGE_SIZE));

    if (mprotect(arena, page_size, PROT_READ | PROT_WRITE) == -1)
    {
        capy_log_errno(errno, "mprotect failed");
        abort();
    }

    arena->size = 40;
    arena->capacity = page_size;
    arena->page_size = page_size;
    arena->limit = limit;

    return arena;
}

void capy_arena_free(capy_arena *arena)
{
    if (munmap(arena, arena->limit) == -1)
    {
        capy_log_errno(errno, "munmap failed");
        abort();
    }
}

void *capy_arena_grow(capy_arena *arena, size_t size, size_t align, int zeroinit)
{
    capy_assert(arena != NULL);

    size_t begin = (align) ? align_to(arena->size, align) : arena->size;
    size_t end = begin + size;

    if (end > arena->limit)
    {
        capy_log_errno(ENOMEM, "limit reached");
        abort();
    }

    if (end > arena->capacity)
    {
        size_t capacity = align_to(end, arena->page_size);

        if (mprotect(arena, capacity, PROT_READ | PROT_WRITE) < 0)
        {
            capy_log_errno(errno, "mprotect failed");
            abort();
        }

        arena->capacity = capacity;
    }

    arena->size = end;

    void *data = (uint8_t *)(arena) + begin;

    if (zeroinit)
    {
        memset(data, 0, size);
    }

    return data;
}

size_t capy_arena_size(capy_arena *arena)
{
    return arena->size;
}

void *capy_arena_top(capy_arena *arena)
{
    return (uint8_t *)(arena) + arena->size;
}

void capy_arena_shrink(capy_arena *arena, void *addr)
{
    capy_assert((size_t)(addr) >= (size_t)(arena) + sizeof(capy_arena));
    capy_assert((size_t)(addr) <= (size_t)(arena) + arena->size);
    arena->size = (size_t)(addr) - (size_t)(arena);
}
