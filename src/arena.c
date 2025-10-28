#include <capy/macros.h>

struct capy_arena
{
    size_t used;
    size_t capacity;
    size_t min;
    size_t max;
    size_t size;
    size_t page_size;
};

Platform static capy_arena *arena_init(size_t min, size_t max);
Platform static capy_err arena_destroy(capy_arena *arena);
Platform static void *arena_create_stack(capy_arena *arena, size_t size);
Platform static void *arena_alloc(capy_arena *arena, size_t size, size_t align, int zeroinit);
Platform static capy_err arena_free(capy_arena *arena, void *addr);

// INTERNAL VARIABLES

static atomic_int arena_allocs = 0;

// PUBLIC DEFINITIONS

capy_arena *capy_arena_init(size_t min, size_t max)
{
    return arena_init(min, max);
}

capy_err capy_arena_destroy(capy_arena *arena)
{
    return arena_destroy(arena);
}

void *capy_arena_create_stack(capy_arena *arena, size_t size)
{
    return arena_create_stack(arena, size);
}

void *capy_arena_alloc(capy_arena *arena, size_t size, size_t align, int zeroinit)
{
    return arena_alloc(arena, size, align, zeroinit);
}

capy_err capy_arena_free(capy_arena *arena, void *addr)
{
    return arena_free(arena, addr);
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

size_t capy_arena_available(capy_arena *arena)
{
    return arena->max - arena->used;
}

size_t capy_arena_used(capy_arena *arena)
{
    return arena->used;
}

void *capy_arena_end(capy_arena *arena)
{
    return Cast(char *, arena) + arena->used;
}

//
// LINUX
//

#ifdef CAPY_OS_LINUX

#include <sys/mman.h>
#include <unistd.h>

Linux static capy_arena *arena_init(size_t min, size_t max)
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
        munmap(arena, max);
        return NULL;
    }

    int count = atomic_fetch_add(&arena_allocs, 1);
    LogMem("capy_arena_init: capacity=%zu count=%d", min, count + 1);

    arena->used = sizeof(capy_arena);
    arena->capacity = min;
    arena->page_size = page_size;
    arena->min = min;
    arena->max = max;
    arena->size = max;

    return arena;
}

Linux static capy_err arena_destroy(capy_arena *arena)
{
    capy_assert(arena != NULL);

    if (munmap(arena, arena->size) == -1)
    {
        return ErrStd(errno);
    }

    int count = atomic_fetch_sub(&arena_allocs, 1);
    LogMem("capy_arena_destroy: count=%d", count - 1);

    return Ok;
}

Linux static void *arena_create_stack(capy_arena *arena, size_t size)
{
    size = align_to(size, arena->page_size);

    if (arena->max - arena->used < (size + arena->page_size))
    {
        return NULL;
    }

    char *stack = Cast(char *, arena) + arena->max - size;

    if (mprotect(stack, size, PROT_READ | PROT_WRITE) == -1)
    {
        return NULL;
    }

    if (mprotect(stack - arena->page_size, arena->page_size, PROT_NONE) == -1)
    {
        return NULL;
    }

    arena->max -= size + arena->page_size;

    return stack + size;
}

Linux static void *arena_alloc(capy_arena *arena, size_t size, size_t align, int zeroinit)
{
    capy_assert(arena != NULL);

    size_t begin = (align) ? align_to(arena->used, align) : arena->used;
    size_t end = begin + size;

    if (end > arena->max)
    {
        return NULL;
    }

    if (end > arena->capacity)
    {
        size_t capacity = next_pow2(end);

        if (capacity > arena->max)
        {
            capacity = arena->max;
        }

        if (mprotect(arena, capacity, PROT_READ | PROT_WRITE) == -1)
        {
            return NULL;
        }

        LogMem("capy_arena_alloc: ptr=%p from=%zu to=%zu", (void *)arena, arena->capacity, capacity);

        arena->capacity = capacity;
    }

    arena->used = end;

    char *data = Cast(char *, arena) + begin;

    if (zeroinit)
    {
        memset(data, 0, size);
    }

    return data;
}

Linux static capy_err arena_free(capy_arena *arena, void *addr)
{
    capy_assert(Cast(uintptr_t, addr) >= Cast(uintptr_t, arena) + sizeof(capy_arena));
    capy_assert(Cast(uintptr_t, addr) <= Cast(uintptr_t, arena) + arena->used);

    arena->used = Cast(size_t, Cast(char *, addr) - Cast(char *, arena));

    if (arena->capacity > arena->min)
    {
        size_t threshold = arena->capacity >> 2;

        if (arena->used <= threshold)
        {
            size_t capacity = next_pow2(arena->used << 1);

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

#endif
