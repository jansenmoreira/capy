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

// INTERNAL DEFINITIONS

static void task_destroy(void)
{
    taskscheduler_done();
}

static taskqueue *taskqueue_init(capy_arena *arena)
{
    size_t capacity = 8;

    char *addr = capy_arena_alloc(arena, sizeof(taskqueue) + (capacity * sizeof(capy_task *)), 8, false);

    if (addr == NULL)
    {
        return NULL;
    }

    taskqueue *queue = Cast(taskqueue *, addr);

    queue->size = 0;
    queue->capacity = capacity;
    queue->element_size = sizeof(capy_task *);
    queue->arena = arena;
    queue->data = Cast(capy_task **, addr + sizeof(taskqueue));

    return queue;
}

static int64_t taskqueue_cmp(taskqueue *queue, size_t a, size_t b)
{
    return capy_timespec_diff(queue->data[a]->deadline, queue->data[b]->deadline);
}

static void taskqueue_swap(taskqueue *queue, size_t a, size_t b)
{
    capy_task *tmp = queue->data[a];
    queue->data[a] = queue->data[b];
    queue->data[a]->queuepos = a;
    queue->data[b] = tmp;
    queue->data[b]->queuepos = b;
}

static void taskqueue_siftup(taskqueue *queue, size_t node)
{
    while (node > 0)
    {
        size_t parent = (node - 1) / 2;

        if (taskqueue_cmp(queue, parent, node) < 0)
        {
            return;
        }

        taskqueue_swap(queue, parent, node);
        node = parent;
    }
}

static void taskqueue_siftdown(taskqueue *queue, size_t node)
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

        if ((right < size) && taskqueue_cmp(queue, right, left) < 0)
        {
            selected = right;
        }

        if (taskqueue_cmp(queue, selected, node) >= 0)
        {
            return;
        }

        taskqueue_swap(queue, selected, node);
        node = selected;
    }
}

static capy_err taskqueue_add(taskqueue *queue, capy_task *value)
{
    size_t pos = queue->size;

    capy_err err = capy_vec_insert(queue->arena, &queue->vec, queue->size, 1, &value);

    if (err.code)
    {
        return err;
    }

    value->queuepos = pos;
    taskqueue_siftup(queue, pos);

    return Ok;
}

static capy_task *taskqueue_peek(taskqueue *queue)
{
    if (queue->size == 0)
    {
        return NULL;
    }

    return queue->data[0];
}

static capy_task *taskqueue_remove(taskqueue *queue, size_t node)
{
    if (node >= queue->size)
    {
        return NULL;
    }

    capy_task *value = queue->data[node];

    queue->data[node] = queue->data[queue->size - 1];
    queue->data[node]->queuepos = node;
    queue->size -= 1;

    taskqueue_siftdown(queue, node);

    value->queuepos = TASKQUEUE_REMOVED;

    return value;
}

static capy_task *taskqueue_pop(taskqueue *queue)
{
    return taskqueue_remove(queue, 0);
}

static int64_t taskqueue_timeout(taskqueue *queue)
{
    int64_t ns = capy_timespec_diff(queue->data[0]->deadline, capy_now());
    return ns / 1000 / 1000;
}

// PUBLIC DEFINITIONS

capy_task *capy_task_init(capy_arena *arena, size_t size, void (*entrypoint)(void), void *ctx)
{
    uintptr_t *stack = capy_arena_create_stack(arena, size);

    if (stack == NULL)
    {
        return NULL;
    }

    *(--stack) = Cast(uintptr_t, task_destroy);
    *(--stack) = Cast(uintptr_t, entrypoint);

    capy_task *task = Make(arena, capy_task, 1);

    if (task == NULL)
    {
        return NULL;
    }

    task_set_stack(&task->abi, stack);
    task->ctx = ctx;
    task->queuepos = TASKQUEUE_REMOVED;

    return task;
}

void capy_task_set_cleanup(capy_task *task, void (*cleanup)(void *))
{
    task->cleanup = cleanup;
}

void *capy_task_ctx(capy_task *task)
{
    return task->ctx;
}

capy_err capy_task_enqueue(capy_task *task, capy_fd fd, bool write, uint64_t timeout)
{
    struct timespec deadline = capy_timespec_addms(capy_now(), timeout);

    task->fd = fd;
    task->write = write;
    task->deadline = deadline;

    capy_err err = taskscheduler_subscribe(task);

    if (err.code)
    {
        return err;
    }

    return taskscheduler_enqueue(task);
}

capy_err capy_task_waitfd(capy_fd fd, bool write, uint64_t timeout)
{
    capy_task *task = NULL;

    capy_err err = taskscheduler_active(&task);

    if (err.code)
    {
        return err;
    }

    task->fd = fd;
    task->write = write;
    task->deadline = capy_timespec_addms(capy_now(), timeout);

    err = taskscheduler_subscribe(task);

    if (err.code)
    {
        return err;
    }

    err = taskscheduler_enqueue(task);

    if (err.code)
    {
        return err;
    }

    err = taskscheduler_wait();

    if (err.code)
    {
        return err;
    }

    if (capy_timespec_diff(capy_task_active()->deadline, capy_now()) <= 0)
    {
        return ErrStd(ETIMEDOUT);
    }

    return Ok;
}

capy_err capy_task_sleep(uint64_t ms)
{
    capy_task *task = NULL;

    capy_err err = taskscheduler_active(&task);

    if (err.code)
    {
        return err;
    }

    task->fd = -1;
    task->deadline = capy_timespec_addms(capy_now(), ms);

    err = taskscheduler_enqueue(task);

    if (err.code)
    {
        return err;
    }

    err = taskscheduler_wait();

    if (err.code)
    {
        return err;
    }

    return Ok;
}

capy_task *capy_task_active(void)
{
    capy_task *task = NULL;

    capy_err err = taskscheduler_active(&task);

    if (err.code)
    {
        return NULL;
    }

    return task;
}

bool capy_task_canceled(void)
{
    return taskscheduler_canceled();
}

capy_err capy_tasks_join(Unused int timeout)
{
    return taskscheduler_join(timeout);
}
