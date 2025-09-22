#include <capy/capy.h>
#include <capy/macros.h>
#include <sys/mman.h>
#include <unistd.h>

// TYPES

struct capy_arena
{
    size_t size;
    size_t capacity;
    size_t min;
    size_t max;
    size_t page_size;
};

// DEFINITIONS

capy_arena *capy_arena_init(size_t min, size_t max)
{
    size_t page_size = Cast(size_t, sysconf(_SC_PAGE_SIZE));

    max = align_to(max, page_size);
    min = (min != 0) ? align_to(min, page_size) : page_size;

    capy_arena *arena = mmap(NULL, max, PROT_NONE, MAP_NORESERVE | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (arena == MAP_FAILED)
    {
        return NULL;
    }

    if (mprotect(arena, min, PROT_READ | PROT_WRITE) == -1)
    {
        return NULL;
    }

    LogMem("capy_arena_init: ptr=%p capacity=%zu", (void *)arena, min);

    arena->size = 40;
    arena->capacity = min;
    arena->page_size = page_size;
    arena->min = min;
    arena->max = max;

    return arena;
}

capy_err capy_arena_destroy(capy_arena *arena)
{
    capy_assert(arena != NULL);

    if (munmap(arena, arena->max) == -1)
    {
        return ErrStd(errno);
    }

    return Ok;
}

void *capy_arena_realloc(capy_arena *arena, void *data, size_t size, size_t new_size, int zeroinit)
{
    capy_assert(arena != NULL);
    capy_assert(data != NULL);
    capy_assert(new_size > size);

    void *tmp;

    char *end = Cast(char *, data) + size;

    if (end == capy_arena_end(arena))
    {
        tmp = capy_arena_alloc(arena, new_size - size, 0, zeroinit);

        if (tmp == NULL)
        {
            return NULL;
        }
    }
    else
    {
        tmp = capy_arena_alloc(arena, new_size, 8, zeroinit);

        if (tmp == NULL)
        {
            return NULL;
        }

        memcpy(tmp, data, size);
        data = tmp;
    }

    return data;
}

void *capy_arena_alloc(capy_arena *arena, size_t size, size_t align, int zeroinit)
{
    capy_assert(arena != NULL);

    size_t begin = (align) ? align_to(arena->size, align) : arena->size;
    size_t end = begin + size;

    if (end > arena->max)
    {
        return NULL;
    }

    if (end > arena->capacity)
    {
        size_t capacity = next_pow2(end);

        if (mprotect(arena, capacity, PROT_READ | PROT_WRITE))
        {
            return NULL;
        }

        LogMem("capy_arena_alloc: ptr=%p from=%zu to=%zu", (void *)arena, arena->capacity, capacity);

        arena->capacity = capacity;
    }

    arena->size = end;

    char *data = Cast(char *, arena) + begin;

    if (zeroinit)
    {
        memset(data, 0, size);
    }

    return data;
}

capy_err capy_arena_free(capy_arena *arena, void *addr)
{
    capy_assert(Cast(uintptr_t, addr) >= Cast(uintptr_t, arena) + sizeof(capy_arena));
    capy_assert(Cast(uintptr_t, addr) <= Cast(uintptr_t, arena) + arena->size);

    arena->size = Cast(size_t, Cast(char *, addr) - Cast(char *, arena));

    if (arena->capacity > arena->min)
    {
        size_t threshold = arena->capacity >> 2;

        if (arena->size <= threshold)
        {
            size_t capacity = next_pow2(arena->size << 1);

            if (capacity < arena->min)
            {
                capacity = arena->min;
            }

            char *tail = Cast(char *, arena) + capacity;

            size_t tail_size = arena->capacity - capacity;

            if (mprotect(tail, tail_size, PROT_NONE))
            {
                return ErrStd(errno);
            }

            LogMem("capy_arena_free: ptr=%p from=%zu to=%zu", (void *)arena, arena->capacity, capacity);

            arena->capacity = capacity;
        }
    }

    return Ok;
}

size_t capy_arena_size(capy_arena *arena)
{
    capy_assert(arena != NULL);
    return arena->size;
}

void *capy_arena_end(capy_arena *arena)
{
    capy_assert(arena != NULL);
    return Cast(char *, arena) + arena->size;
}
