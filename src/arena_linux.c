#include <capy/capy.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

// TYPES

struct capy_arena
{
    size_t size;
    size_t capacity;
    size_t limit;
    size_t page_size;
};

// DEFINITIONS

capy_arena *capy_arena_init(size_t limit)
{
    size_t page_size = (size_t)(sysconf(_SC_PAGE_SIZE));

    limit = align_to(limit, page_size);

    capy_arena *arena = mmap(NULL, limit, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (arena == MAP_FAILED)
    {
        return NULL;
    }

    if (mprotect(arena, page_size, PROT_READ | PROT_WRITE) == -1)
    {
        return NULL;
    }

    arena->size = 40;
    arena->capacity = page_size;
    arena->page_size = page_size;
    arena->limit = limit;

    return arena;
}

void capy_arena_destroy(capy_arena *arena)
{
    capy_assert(arena != NULL);
    munmap(arena, arena->limit);
}

void *capy_arena_realloc(capy_arena *arena, void *src, size_t size, size_t new_size, int zeroinit)
{
    capy_assert(arena != NULL);
    capy_assert(src != NULL);
    capy_assert(new_size > size);

    char *end = cast(char *, src) + size;

    if (end == capy_arena_end(arena))
    {
        if (capy_arena_alloc(arena, new_size - size, 0, zeroinit) == NULL)
        {
            return NULL;
        }

        return src;
    }

    char *dest = capy_arena_alloc(arena, new_size, 8, false);

    if (dest != NULL)
    {
        memcpy(dest, src, size);
    }

    return dest;
}

void *capy_arena_alloc(capy_arena *arena, size_t size, size_t align, int zeroinit)
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
        size_t capacity = arena->capacity << 1;

        if (mprotect(arena, capacity, PROT_READ | PROT_WRITE))
        {
            return NULL;
        }

        arena->capacity = capacity;
    }

    arena->size = end;

    char *data = (char *)(arena) + begin;

    if (zeroinit)
    {
        memset(data, 0, size);
    }

    return data;
}

int capy_arena_free(capy_arena *arena, void *addr)
{
    capy_assert(cast(size_t, addr) >= cast(size_t, arena) + sizeof(capy_arena));
    capy_assert(cast(size_t, addr) <= cast(size_t, arena) + arena->size);
    arena->size = cast(size_t, addr) - cast(size_t, arena);

    size_t threshold = arena->capacity >> 2;
    size_t capacity = arena->capacity >> 1;

    if (arena->size <= threshold)
    {
        char *tail = (char *)(arena) + capacity;

        size_t tail_size = arena->capacity - capacity;

        if (mprotect(tail, tail_size, PROT_NONE))
        {
            return errno;
        }

        arena->capacity = capacity;
    }

    return 0;
}

size_t capy_arena_size(capy_arena *arena)
{
    capy_assert(arena != NULL);
    return arena->size;
}

void *capy_arena_end(capy_arena *arena)
{
    capy_assert(arena != NULL);
    return (char *)(arena) + arena->size;
}
