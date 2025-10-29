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

#define TASKQUEUE_REMOVED Cast(size_t, -1)

// DECLARATIONS

struct task
{
    struct taskctx *ctx;
    void *data;
    void (*entrypoint)(void *ctx);
    void (*cleanup)(void *ctx);
    struct timespec deadline;
    capy_fd fd;
    bool write;
    bool ready;
    size_t queuepos;
};

union taskqueue
{
    capy_vec vec;
    struct
    {
        size_t size;
        size_t capacity;
        size_t element_size;
        struct task **data;
        capy_arena *arena;
    };
};

struct taskscheduler
{
    bool canceled;
    capy_err err;
    capy_arena *arena;
    struct taskpoll *poll;
    struct task *main;
    struct task *poller;
    struct task *cleaner;
    struct task *active;
    struct task *previous;
    union taskqueue *queue;
};

Platform static size_t task_thread_id(void);
Platform static size_t task_ncpus(void);
Platform static void task_cancel(void);

Platform static void taskpoll_wait(void *data);
Platform static capy_err taskpoll_init(struct taskscheduler *scheduler);
Platform static capy_err taskpoll_add(struct taskscheduler *scheduler, struct task *task);
Platform static capy_err taskpoll_remove(struct taskscheduler *scheduler, struct task *task);
Platform static capy_err taskpoll_destroy(struct taskscheduler *scheduler);
Platform static struct taskctx *taskctx_init(capy_arena *arena, void *stack, void (*entrypoint)(void));
Platform static void task_switch(struct taskctx *next, struct taskctx *current);

static void scheduler_switch(struct taskscheduler *scheduler, struct task *task);
static capy_err scheduler_init(void);
static capy_err scheduler_shutdown(struct taskscheduler *scheduler, uint64_t timeout);
static capy_err scheduler_waitfd(struct taskscheduler *scheduler, struct task *task, capy_fd fd, bool write, uint64_t timeout);
static capy_err scheduler_sleep(struct taskscheduler *scheduler, struct task *task, uint64_t timeout);
static void scheduler_clean(void *data);

static struct task *task_init(capy_arena *arena, size_t size, void (*entrypoint)(void *data), void (*cleanup)(void *data), void *data);
static void task_entrypoint(void);

static union taskqueue *taskqueue_init(capy_arena *arena);
static int64_t taskqueue_cmp(union taskqueue *queue, size_t a, size_t b);
static void taskqueue_swap(union taskqueue *queue, size_t a, size_t b);
static void taskqueue_siftup(union taskqueue *queue, size_t node);
static void taskqueue_siftdown(union taskqueue *queue, size_t node);
static capy_err taskqueue_add(union taskqueue *queue, struct task *value);
static struct task *taskqueue_remove(union taskqueue *queue, size_t node);
static struct task *taskqueue_pop(union taskqueue *queue);
static int64_t taskqueue_timeout(union taskqueue *queue);

// INTERNAL VARIABLES

static thread_local struct taskscheduler *task_scheduler = NULL;

// PUBLIC DEFINITINOS

capy_err capy_task_init(capy_arena *arena, size_t size, void (*entrypoint)(void *ctx), void (*cleanup)(void *ctx), void *data)
{
    capy_err err = scheduler_init();

    if (err.code)
    {
        return err;
    }

    struct task *task = task_init(arena, size, entrypoint, cleanup, data);

    if (task == NULL)
    {
        return ErrStd(ENOMEM);
    }

    task->deadline = capy_now();

    return taskqueue_add(task_scheduler->queue, task);
}

capy_err capy_waitfd(capy_fd fd, bool write, uint64_t timeout)
{
    capy_err err = scheduler_init();

    if (err.code)
    {
        return err;
    }

    struct task *task = task_scheduler->active;

    err = scheduler_waitfd(task_scheduler, task, fd, write, timeout);

    if (err.code)
    {
        return err;
    }

    scheduler_switch(task_scheduler, task_scheduler->poller);

    if (capy_timespec_diff(task->deadline, capy_now()) <= 0)
    {
        return ErrStd(ETIMEDOUT);
    }

    if (task == task_scheduler->main)
    {
        if (task_scheduler->err.code)
        {
            return err;
        }

        if (capy_canceled())
        {
            return ErrStd(ECANCELED);
        }
    }

    return Ok;
}

capy_err capy_sleep(uint64_t ms)
{
    capy_err err = scheduler_init();

    if (err.code)
    {
        return err;
    }

    err = scheduler_sleep(task_scheduler, task_scheduler->active, ms);

    if (err.code)
    {
        return err;
    }

    scheduler_switch(task_scheduler, task_scheduler->poller);

    return Ok;
}

capy_err capy_shutdown(uint64_t timeout)
{
    if (task_scheduler == NULL)
    {
        return Ok;
    }

    return scheduler_shutdown(task_scheduler, timeout);
}

void capy_cancel(void)
{
    task_cancel();
}

bool capy_canceled(void)
{
    if (task_scheduler == NULL)
    {
        return false;
    }

    return task_scheduler->canceled;
}

size_t capy_thread_id(void)
{
    return task_thread_id();
}

size_t capy_ncpus(void)
{
    return task_ncpus();
}

// INTERNAL DEFINITIONS

static union taskqueue *taskqueue_init(capy_arena *arena)
{
    size_t capacity = 8;

    char *addr = capy_arena_alloc(arena, sizeof(union taskqueue) + (capacity * sizeof(struct task *)), 8, false);

    if (addr == NULL)
    {
        return NULL;
    }

    union taskqueue *queue = Cast(union taskqueue *, addr);

    queue->size = 0;
    queue->capacity = capacity;
    queue->element_size = sizeof(struct task *);
    queue->arena = arena;
    queue->data = Cast(struct task **, addr + sizeof(union taskqueue));

    return queue;
}

static int64_t taskqueue_cmp(union taskqueue *queue, size_t a, size_t b)
{
    return capy_timespec_diff(queue->data[a]->deadline, queue->data[b]->deadline);
}

static void taskqueue_swap(union taskqueue *queue, size_t a, size_t b)
{
    struct task *tmp = queue->data[a];
    queue->data[a] = queue->data[b];
    queue->data[a]->queuepos = a;
    queue->data[b] = tmp;
    queue->data[b]->queuepos = b;
}

static void taskqueue_siftup(union taskqueue *queue, size_t node)
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

static void taskqueue_siftdown(union taskqueue *queue, size_t node)
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

static capy_err taskqueue_add(union taskqueue *queue, struct task *value)
{
    if (value->queuepos != TASKQUEUE_REMOVED)
    {
        taskqueue_remove(queue, value->queuepos);
    }

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

static struct task *taskqueue_remove(union taskqueue *queue, size_t node)
{
    if (node >= queue->size)
    {
        return NULL;
    }

    taskqueue_swap(queue, node, queue->size - 1);
    queue->size -= 1;

    if (node != 0 && taskqueue_cmp(queue, node, (node - 1) / 2) < 0)
    {
        taskqueue_siftup(queue, node);
    }
    else
    {
        taskqueue_siftdown(queue, node);
    }

    struct task *value = queue->data[queue->size];
    value->queuepos = TASKQUEUE_REMOVED;

    return value;
}

static struct task *taskqueue_pop(union taskqueue *queue)
{
    return taskqueue_remove(queue, 0);
}

static int64_t taskqueue_timeout(union taskqueue *queue)
{
    return capy_timespec_diff(queue->data[0]->deadline, capy_now()) / 1000 / 1000;
}

static void task_entrypoint(void)
{
    struct task *task = task_scheduler->active;
    task->entrypoint(task->data);
    scheduler_switch(task_scheduler, task_scheduler->cleaner);
}

// SCHEDULER

static capy_err scheduler_init(void)
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

    task_scheduler = Make(arena, struct taskscheduler, 1);

    if (task_scheduler == NULL)
    {
        capy_arena_destroy(arena);
        return ErrStd(ENOMEM);
    }

    task_scheduler->arena = arena;

    task_scheduler->main = Make(arena, struct task, 1);

    if (task_scheduler->main == NULL)
    {
        return ErrStd(ENOMEM);
    }

    task_scheduler->main->ctx = taskctx_init(arena, NULL, NULL);

    if (task_scheduler->main->ctx == NULL)
    {
        capy_arena_destroy(arena);
        return ErrStd(ENOMEM);
    }

    task_scheduler->active = task_scheduler->main;

    task_scheduler->poller = task_init(arena, KiB(64), taskpoll_wait, NULL, task_scheduler);

    if (task_scheduler->poller == NULL)
    {
        capy_arena_destroy(arena);
        return ErrStd(ENOMEM);
    }

    task_scheduler->cleaner = task_init(arena, KiB(32), scheduler_clean, NULL, task_scheduler);

    if (task_scheduler->cleaner == NULL)
    {
        capy_arena_destroy(arena);
        return ErrStd(ENOMEM);
    }

    task_scheduler->queue = taskqueue_init(arena);

    if (task_scheduler->queue == NULL)
    {
        capy_arena_destroy(arena);
        return ErrStd(ENOMEM);
    }

    capy_err err = taskpoll_init(task_scheduler);

    if (err.code)
    {
        capy_arena_destroy(arena);
        return err;
    }

    return Ok;
}

static void scheduler_clean(void *data)
{
    struct taskscheduler *scheduler = data;

    for (;;)
    {
        if (scheduler->previous->cleanup != NULL)
        {
            scheduler->previous->cleanup(scheduler->previous->data);
        }

        scheduler_switch(scheduler, scheduler->poller);
    }
}

static void scheduler_switch(struct taskscheduler *scheduler, struct task *task)
{
    scheduler->previous = scheduler->active;
    scheduler->previous->ready = false;
    scheduler->active = task;
    task_switch(scheduler->active->ctx, scheduler->previous->ctx);
}

static capy_err scheduler_shutdown(struct taskscheduler *scheduler, uint64_t timeout)
{
    if (scheduler->active != scheduler->main)
    {
        return ErrStd(EINVAL);
    }

    capy_sleep(timeout);
    taskpoll_destroy(scheduler);
    capy_arena_destroy(scheduler->arena);

    return Ok;
}

static capy_err scheduler_waitfd(struct taskscheduler *scheduler, struct task *task, capy_fd fd, bool write, uint64_t timeout)
{
    task->fd = fd;
    task->write = write;

    if (timeout == 0)
    {
        timeout = Years(50ull);
    }

    task->deadline = capy_timespec_addms(capy_now(), timeout);

    capy_err err = taskqueue_add(scheduler->queue, task);

    if (err.code)
    {
        return err;
    }

    err = taskpoll_add(task_scheduler, task);

    if (err.code)
    {
        return err;
    }

    return Ok;
}

static capy_err scheduler_sleep(struct taskscheduler *scheduler, struct task *task, uint64_t timeout)
{
    task->deadline = capy_now();

    if (timeout != 0)
    {
        task->deadline = capy_timespec_addms(task->deadline, timeout);
    }

    return taskqueue_add(scheduler->queue, task);
}

static struct task *task_init(capy_arena *arena, size_t size, void (*entrypoint)(void *ctx), void (*cleanup)(void *ctx), void *data)
{
    uintptr_t *stack = capy_arena_create_stack(arena, size);

    if (stack == NULL)
    {
        return NULL;
    }

    struct task *task = Make(arena, struct task, 1);

    if (task == NULL)
    {
        return NULL;
    }

    task->ctx = taskctx_init(arena, stack, task_entrypoint);

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

//
// LINUX DEFINITIONS
//

#ifdef CAPY_OS_LINUX

#include <pthread.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>

struct taskpoll
{
    int fd;
    int signal_fd;
};

Linux static void taskpoll_wait(void *data)
{
    struct taskscheduler *scheduler = data;

    struct task *ready[32];
    int ready_count = 0;
    int ready_max = ArrLen(ready);
    int timeout_max = ready_max / 2;
    struct epoll_event events[ready_max];

    for (;;)
    {
        if (scheduler->canceled && scheduler->queue->size == 1)
        {
            scheduler_switch(scheduler, scheduler->main);
        }

        ready_count = 0;

        int timeout = Seconds(10);

        while (scheduler->queue->size > 0 && ready_count < timeout_max)
        {
            int64_t ms = taskqueue_timeout(scheduler->queue);

            if (ms <= 0)
            {
                struct task *task = taskqueue_pop(scheduler->queue);
                scheduler->err = taskpoll_remove(scheduler, task);

                if (scheduler->err.code)
                {
                    scheduler_switch(scheduler, scheduler->main);
                }

                if (!task->ready)
                {
                    ready[ready_count++] = task;
                    task->ready = true;
                }

                continue;
            }

            if (ms < timeout)
            {
                timeout = Cast(int, ms) + 1;
            }

            break;
        }

        if (ready_count > 0)
        {
            timeout = 0;
        }

        int available = ready_max - ready_count;

        if (available)
        {
            int count = epoll_wait(scheduler->poll->fd, events, available, timeout);

            if (count == -1)
            {
                capy_err err = ErrStd(errno);

                if (err.code == EINTR)
                {
                    continue;
                }

                scheduler->err = err;
                scheduler_switch(scheduler, scheduler->main);
                continue;
            }

            for (int i = 0; i < count; i++)
            {
                struct task *task = NULL;

                if (events[i].data.u64 == 0)
                {
                    close(scheduler->poll->signal_fd);
                    scheduler->canceled = true;
                    task = scheduler->main;
                }
                else
                {
                    task = events[i].data.ptr;
                }

                taskqueue_remove(scheduler->queue, task->queuepos);

                if (!task->ready)
                {
                    ready[ready_count++] = task;
                    task->ready = true;
                }
            }
        }

        for (int i = 0; i < ready_count; i++)
        {
            scheduler_switch(scheduler, ready[i]);
        }
    }
}

Linux static capy_err taskpoll_init(struct taskscheduler *scheduler)
{
    struct taskpoll *poll = Make(scheduler->arena, struct taskpoll, 1);

    if (poll == NULL)
    {
        return ErrStd(ENOMEM);
    }

    poll->fd = epoll_create1(0);

    if (poll->fd == -1)
    {
        return ErrStd(errno);
    }

    sigset_t signals;

    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &signals, NULL);

    int signal_fd = signalfd(-1, &signals, 0);

    if (signal_fd == -1)
    {
        close(poll->fd);
        return ErrStd(errno);
    }

    struct epoll_event event = {
        .events = EPOLLIN | EPOLLET | EPOLLONESHOT,
        .data.u64 = 0,
    };

    if (epoll_ctl(poll->fd, EPOLL_CTL_ADD, signal_fd, &event) == -1)
    {
        close(poll->fd);
        close(poll->signal_fd);
        return ErrStd(errno);
    }

    scheduler->poll = poll;

    return Ok;
}

Linux static capy_err taskpoll_destroy(struct taskscheduler *scheduler)
{
    close(scheduler->poll->fd);
    close(scheduler->poll->signal_fd);
    return Ok;
}

Linux static capy_err taskpoll_remove(struct taskscheduler *scheduler, struct task *task)
{
    if (task->fd != -1)
    {
        if (epoll_ctl(scheduler->poll->fd, EPOLL_CTL_DEL, task->fd, NULL) == -1)
        {
            capy_err err = ErrStd(errno);

            if (err.code != ENOENT)
            {
                return err;
            }
        }
    }

    return Ok;
}

Linux static capy_err taskpoll_add(struct taskscheduler *scheduler, struct task *task)
{
    if (task->fd == -1)
    {
        return Ok;
    }

    struct epoll_event event = {
        .events = ((task->write) ? EPOLLOUT : EPOLLIN) | EPOLLRDHUP | EPOLLET | EPOLLONESHOT,
        .data.ptr = task,
    };

    if (epoll_ctl(scheduler->poll->fd, EPOLL_CTL_MOD, task->fd, &event) == -1)
    {
        capy_err err = ErrStd(errno);

        if (err.code != ENOENT)
        {
            return err;
        }

        if (epoll_ctl(scheduler->poll->fd, EPOLL_CTL_ADD, task->fd, &event) == -1)
        {
            return ErrStd(errno);
        }
    }

    return Ok;
}

Linux static size_t task_thread_id(void)
{
    return pthread_self();
}

Linux static size_t task_ncpus(void)
{
    return (size_t)(sysconf(_SC_NPROCESSORS_CONF));
}

Linux static void task_cancel(void)
{
    kill(getpid(), SIGTERM);
}

#endif

//
// LINUX AMD64 DEFINITIONS
//

#ifdef CAPY_LINUX_AMD64

struct taskctx
{
    uint64_t rsp;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
};

LinuxAmd64 static struct taskctx *taskctx_init(capy_arena *arena, void *stack_, void (*entrypoint)(void))
{
    struct taskctx *ctx = Make(arena, struct taskctx, 1);

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

LinuxAmd64 static __attribute__((naked)) void task_switch(Unused struct taskctx *next, Unused struct taskctx *current)
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

#endif
