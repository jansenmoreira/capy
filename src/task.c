#include "capy.h"

// INTERNAL VARIABLES

static thread_local capy_scheduler *task_scheduler = NULL;

// INTERNAL DEFINITIONS

capy_taskqueue *capy_taskqueue_init_(capy_arena *arena)
{
    size_t capacity = 8;

    char *addr = capy_arena_alloc(arena, sizeof(capy_taskqueue) + (capacity * sizeof(capy_task *)), 8, false);

    if (addr == NULL)
    {
        return NULL;
    }

    capy_taskqueue *queue = Cast(capy_taskqueue *, addr);

    queue->size = 0;
    queue->capacity = capacity;
    queue->element_size = sizeof(capy_task *);
    queue->arena = arena;
    queue->data = Cast(capy_task **, addr + sizeof(capy_taskqueue));

    return queue;
}

int64_t capy_taskqueue_cmp_(capy_taskqueue *queue, size_t a, size_t b)
{
    return capy_timespec_diff(queue->data[a]->deadline, queue->data[b]->deadline);
}

void capy_taskqueue_swap_(capy_taskqueue *queue, size_t a, size_t b)
{
    capy_task *tmp = queue->data[a];
    queue->data[a] = queue->data[b];
    queue->data[a]->queuepos = a;
    queue->data[b] = tmp;
    queue->data[b]->queuepos = b;
}

void capy_taskqueue_siftup_(capy_taskqueue *queue, size_t node)
{
    while (node > 0)
    {
        size_t parent = (node - 1) / 2;

        if (capy_taskqueue_cmp_(queue, parent, node) < 0)
        {
            return;
        }

        capy_taskqueue_swap_(queue, parent, node);
        node = parent;
    }
}

void capy_taskqueue_siftdown_(capy_taskqueue *queue, size_t node)
{
    size_t size = queue->size;

    for (;;)
    {
        size_t left = 2 * node + 1;
        size_t right = 2 * node + 2;

        if (left >= size)
        {
            return;
        }

        size_t selected = left;

        if ((right < size) && capy_taskqueue_cmp_(queue, right, left) < 0)
        {
            selected = right;
        }

        if (capy_taskqueue_cmp_(queue, selected, node) >= 0)
        {
            return;
        }

        capy_taskqueue_swap_(queue, selected, node);
        node = selected;
    }
}

capy_err capy_taskqueue_add_(capy_taskqueue *queue, capy_task *value)
{
    if (value->queuepos != TASKQUEUE_REMOVED)
    {
        capy_taskqueue_remove_(queue, value->queuepos);
    }

    size_t pos = queue->size;

    capy_err err = capy_vec_insert(queue->arena, &queue->vec, queue->size, 1, &value);

    if (err.code)
    {
        return err;
    }

    value->queuepos = pos;
    capy_taskqueue_siftup_(queue, pos);

    return Ok;
}

capy_task *capy_taskqueue_peek_(capy_taskqueue *queue)
{
    if (queue->size == 0)
    {
        return NULL;
    }

    return queue->data[0];
}

capy_task *capy_taskqueue_remove_(capy_taskqueue *queue, size_t node)
{
    if (node >= queue->size)
    {
        return NULL;
    }

    capy_taskqueue_swap_(queue, node, queue->size - 1);
    queue->size -= 1;

    if (node != 0 && capy_taskqueue_cmp_(queue, node, (node - 1) / 2) < 0)
    {
        capy_taskqueue_siftup_(queue, node);
    }
    else
    {
        capy_taskqueue_siftdown_(queue, node);
    }

    capy_task *value = queue->data[queue->size];
    value->queuepos = TASKQUEUE_REMOVED;

    return value;
}

capy_task *capy_taskqueue_pop_(capy_taskqueue *queue)
{
    return capy_taskqueue_remove_(queue, 0);
}

int64_t capy_taskqueue_timeout_(capy_taskqueue *queue)
{
    int64_t ns = capy_timespec_diff(queue->data[0]->deadline, capy_now());
    return ns / 1000 / 1000;
}

void capy_task_entrypoint_(void)
{
    capy_task *task = task_scheduler->active;
    task->entrypoint(task->data);
    capy_scheduler_switch_(task_scheduler, task_scheduler->cleaner);
}

// SCHEDULER

capy_err capy_scheduler_init_(void)
{
    if (task_scheduler != NULL)
    {
        return Ok;
    }

    capy_arena *arena = capy_arena_init(0, MiB(2));

    if (arena == NULL)
    {
        return ErrStd(ENOMEM);
    }

    task_scheduler = Make(arena, capy_scheduler, 1);

    if (task_scheduler == NULL)
    {
        capy_arena_destroy(arena);
        return ErrStd(ENOMEM);
    }

    task_scheduler->arena = arena;

    task_scheduler->main = Make(arena, capy_task, 1);

    if (task_scheduler->main == NULL)
    {
        return ErrStd(ENOMEM);
    }

    task_scheduler->main->ctx = capy_task_ctx_init(arena, NULL, NULL);

    if (task_scheduler->main->ctx == NULL)
    {
        capy_arena_destroy(arena);
        return ErrStd(ENOMEM);
    }

    task_scheduler->active = task_scheduler->main;

    task_scheduler->poller = capy_task_init_(arena, KiB(64), capy_poll_, NULL, task_scheduler);

    if (task_scheduler->poller == NULL)
    {
        capy_arena_destroy(arena);
        return ErrStd(ENOMEM);
    }

    task_scheduler->cleaner = capy_task_init_(arena, KiB(32), capy_scheduler_clean_, NULL, task_scheduler);

    if (task_scheduler->cleaner == NULL)
    {
        capy_arena_destroy(arena);
        return ErrStd(ENOMEM);
    }

    task_scheduler->queue = capy_taskqueue_init_(arena);

    if (task_scheduler->queue == NULL)
    {
        capy_arena_destroy(arena);
        return ErrStd(ENOMEM);
    }

    capy_err err = capy_poll_init_(task_scheduler);

    if (err.code)
    {
        capy_arena_destroy(arena);
        return err;
    }

    return Ok;
}

void capy_scheduler_clean_(void *data)
{
    capy_scheduler *scheduler = data;

    for (;;)
    {
        if (scheduler->previous->cleanup != NULL)
        {
            scheduler->previous->cleanup(scheduler->previous->data);
        }

        capy_scheduler_switch_(scheduler, scheduler->poller);
    }
}

void capy_scheduler_switch_(capy_scheduler *scheduler, capy_task *task)
{
    scheduler->previous = scheduler->active;
    scheduler->active = task;
    capy_task_switch_(scheduler->active->ctx, scheduler->previous->ctx);
}

capy_err capy_scheduler_shutdown_(capy_scheduler *scheduler, Unused int timeout)
{
    if (scheduler->active != scheduler->main)
    {
        return ErrStd(EINVAL);
    }

    while (scheduler->queue->size > 0)
    {
        capy_scheduler_switch_(scheduler, scheduler->poller);
    }

    capy_poll_destroy_(scheduler);
    capy_arena_destroy(scheduler->arena);

    return Ok;
}

capy_err capy_scheduler_waitfd_(capy_scheduler *scheduler, capy_task *task, capy_fd fd, bool write, uint64_t timeout)
{
    task->fd = fd;
    task->write = write;

    if (timeout == 0)
    {
        timeout = Years(50);
    }

    task->deadline = capy_timespec_addms(capy_now(), timeout);

    capy_err err = capy_taskqueue_add_(scheduler->queue, task);

    if (err.code)
    {
        return err;
    }

    err = capy_poll_add_(task_scheduler, task);

    if (err.code)
    {
        return err;
    }

    return Ok;
}

capy_err capy_scheduler_sleep_(capy_scheduler *scheduler, capy_task *task, uint64_t timeout)
{
    task->deadline = capy_now();

    if (timeout != 0)
    {
        task->deadline = capy_timespec_addms(task->deadline, timeout);
    }

    return capy_taskqueue_add_(scheduler->queue, task);
}

capy_task *capy_task_init_(capy_arena *arena, size_t size, void (*entrypoint)(void *ctx), void (*cleanup)(void *ctx), void *data)
{
    uintptr_t *stack = capy_arena_create_stack(arena, size);

    if (stack == NULL)
    {
        return NULL;
    }

    capy_task *task = Make(arena, capy_task, 1);

    if (task == NULL)
    {
        return NULL;
    }

    task->ctx = capy_task_ctx_init(arena, stack, capy_task_entrypoint_);

    if (task->ctx == NULL)
    {
        return NULL;
    }

    task->entrypoint = entrypoint;
    task->cleanup = cleanup;
    task->data = data;
    task->queuepos = TASKQUEUE_REMOVED;

    return task;
}

// PUBLIC DEFINITINOS

capy_err capy_task_init(capy_arena *arena, size_t size, void (*entrypoint)(void *ctx), void (*cleanup)(void *ctx), void *data)
{
    capy_err err = capy_scheduler_init_();

    if (err.code)
    {
        return err;
    }

    capy_task *task = capy_task_init_(arena, size, entrypoint, cleanup, data);

    if (task == NULL)
    {
        return ErrStd(ENOMEM);
    }

    task->deadline = capy_now();

    return capy_taskqueue_add_(task_scheduler->queue, task);
}

capy_err capy_waitfd(capy_fd fd, bool write, uint64_t timeout)
{
    capy_err err = capy_scheduler_init_();

    if (err.code)
    {
        return err;
    }

    capy_task *task = task_scheduler->active;

    err = capy_scheduler_waitfd_(task_scheduler, task, fd, write, timeout);

    if (err.code)
    {
        return err;
    }

    capy_scheduler_switch_(task_scheduler, task_scheduler->poller);

    if (capy_timespec_diff(task->deadline, capy_now()) <= 0)
    {
        return ErrStd(ETIMEDOUT);
    }

    if (task == task_scheduler->main && task_scheduler->cancel)
    {
        return ErrStd(ECANCELED);
    }

    return Ok;
}

capy_err capy_sleep(uint64_t ms)
{
    capy_err err = capy_scheduler_init_();

    if (err.code)
    {
        return err;
    }

    err = capy_scheduler_sleep_(task_scheduler, task_scheduler->active, ms);

    if (err.code)
    {
        return err;
    }

    capy_scheduler_switch_(task_scheduler, task_scheduler->poller);

    return Ok;
}

capy_err capy_shutdown(Unused int timeout)
{
    if (task_scheduler == NULL)
    {
        return Ok;
    }

    return capy_scheduler_shutdown_(task_scheduler, timeout);
}

bool capy_task_canceled(void)
{
    return task_scheduler->cancel;
}
