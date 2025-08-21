#include <assert.h>
#include <capy/capy.h>
#include <errno.h>
#include <stdalign.h>
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
        return MAP_FAILED;
    }

    size_t page_size = (size_t)(sysconf(_SC_PAGE_SIZE));

    if (mprotect(arena, page_size, PROT_READ | PROT_WRITE) < 0)
    {
        return NULL;
    }

    arena->size = 40;
    arena->capacity = page_size;
    arena->page_size = page_size;
    arena->limit = limit;

    return arena;
}

int capy_arena_free(capy_arena *arena)
{
    return munmap(arena, arena->limit), errno;
}

void *capy_arena_grow(capy_arena *arena, size_t size, size_t align)
{
    assert(arena != NULL);

    size_t begin = (align) ? align_to(arena->size, align) : arena->size;
    size_t end = begin + size;

    if (end > arena->capacity)
    {
        size_t capacity = align_to(end, arena->page_size);

        if (mprotect(arena, capacity, PROT_READ | PROT_WRITE) < 0)
        {
            return NULL;
        }

        arena->capacity = capacity;
    }

    arena->size = end;

    uint8_t *data = (uint8_t *)(arena);

    memset(data + begin, 0, size);

    return data + begin;
}

void *capy_arena_top(capy_arena *arena)
{
    return (uint8_t *)(arena) + arena->size;
}

int capy_arena_shrink(capy_arena *arena, void *addr)
{
    ptrdiff_t size = (uint8_t *)(addr) - (uint8_t *)(arena);

    if (size < (ptrdiff_t)(sizeof(capy_arena)) || (size_t)(size) > arena->size)
    {
        return EINVAL;
    }

    arena->size = size;

    return 0;
}
