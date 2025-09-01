#include <capy/capy.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>

struct capy_arena
{
    size_t size;
    size_t capacity;
    size_t limit;
    size_t page_size;
};

static inline size_t align_to(size_t size, size_t page_size)
{
    size_t rem = size % page_size;
    return (rem == 0) ? size : size + page_size - rem;
}

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

int capy_arena_free(capy_arena *arena)
{
    if (munmap(arena, arena->limit) == -1)
    {
        capy_log_errno(errno, "munmap failed");
        abort();
    }

    return 0;
}

void *capy_arena_grow(capy_arena *arena, size_t size, size_t align)
{
    capy_assert(arena != NULL);

    size_t begin = (align) ? align_to(arena->size, align) : arena->size;
    size_t end = begin + size;

    if (end > arena->limit)
    {
        return NULL;
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

    return (uint8_t *)(arena) + begin;
}

size_t capy_arena_size(capy_arena *arena)
{
    return arena->size;
}

void *capy_arena_top(capy_arena *arena)
{
    return (uint8_t *)(arena) + arena->size;
}

int capy_arena_shrink(capy_arena *arena, void *addr)
{
    capy_assert((size_t)(addr) >= (size_t)(arena) + sizeof(capy_arena));
    capy_assert((size_t)(addr) <= (size_t)(arena) + arena->size);
    arena->size = (size_t)(addr) - (size_t)(arena);
    return 0;
}
