#include <capy/macros.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>

#include "capy.h"

#define SIGNAL_EPOLL_EVENT -1ULL

// INTERNAL VARIABLES

static thread_local taskscheduler *task_scheduler = NULL;

// INTERNAL DEFINITIONS

static void taskscheduler_switch(capy_task *handle)
{
    task_scheduler->previous = task_scheduler->active;
    task_scheduler->active = handle;
    task_switch_(task_scheduler->active, task_scheduler->previous);
}

static void taskscheduler_done(void)
{
    taskscheduler_switch(task_scheduler->cleaner);
}

static void taskscheduler_poll(void)
{
    struct epoll_event event[1];

    for (;;)
    {
        int timeout = -1;

        if (task_scheduler->queue->size > 0)
        {
            int64_t ms = taskqueue_timeout(task_scheduler->queue);

            if (ms <= 0)
            {
                capy_task *task = taskqueue_pop(task_scheduler->queue);
                taskscheduler_unsubscribe(task);
                taskscheduler_switch(task);
                timeout = 0;
            }
            else if (ms < INT_MAX)
            {
                timeout = Cast(int, ms) + 1;
            }
        }

        if (task_scheduler->cancel)
        {
            taskscheduler_switch(&task_scheduler->main);
        }

        int count = epoll_wait(task_scheduler->fd, event, 1, timeout);

        if (count == -1)
        {
            capy_err err = ErrStd(errno);

            if (err.code == EINTR)
            {
                continue;
            }

            LogErr("While waiting for events: %s", err.msg);
            continue;
        }
        else if (count > 0)
        {
            capy_task *task = NULL;

            if (event[0].data.u64 == SIGNAL_EPOLL_EVENT)
            {
                task_scheduler->cancel = true;
                task = &task_scheduler->main;
            }
            else if (event[0].data.ptr != NULL)
            {
                task = event[0].data.ptr;
            }

            taskqueue_remove(task_scheduler->queue, task->queuepos);
            taskscheduler_switch(task);
        }
    }
}

static void taskcheduler_clean(void)
{
    for (;;)
    {
        if (task_scheduler->previous->cleanup != NULL)
        {
            task_scheduler->previous->cleanup(task_scheduler->previous->ctx);
        }

        taskscheduler_switch(task_scheduler->poller);
    }
}

static capy_err taskcheduler_init(void)
{
    if (task_scheduler != NULL)
    {
        return Ok;
    }

    capy_arena *arena = capy_arena_init(0, KiB(128));

    if (arena == NULL)
    {
        return ErrStd(ENOMEM);
    }

    task_scheduler = Make(arena, taskscheduler, 1);

    if (task_scheduler == NULL)
    {
        capy_arena_destroy(arena);
        return ErrStd(ENOMEM);
    }

    task_scheduler->poller = capy_task_init(arena, KiB(32), taskscheduler_poll, &task_scheduler);

    if (task_scheduler->poller == NULL)
    {
        capy_arena_destroy(arena);
        return ErrStd(ENOMEM);
    }

    task_scheduler->cleaner = capy_task_init(arena, KiB(32), taskcheduler_clean, &task_scheduler);

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

    task_scheduler->fd = epoll_create1(0);

    if (task_scheduler->fd == -1)
    {
        capy_arena_destroy(arena);
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
        close(task_scheduler->fd);
        capy_arena_destroy(arena);
        return ErrStd(errno);
    }

    struct epoll_event event = {
        .events = EPOLLIN | EPOLLET | EPOLLONESHOT,
        .data.u64 = SIGNAL_EPOLL_EVENT,
    };

    if (epoll_ctl(task_scheduler->fd, EPOLL_CTL_ADD, signal_fd, &event) == -1)
    {
        close(task_scheduler->fd);
        close(task_scheduler->signal_fd);
        capy_arena_destroy(arena);
        return ErrStd(errno);
    }

    task_scheduler->arena = arena;
    task_scheduler->active = &task_scheduler->main;
    task_scheduler->cancel = false;

    return Ok;
}

static capy_err taskscheduler_unsubscribe(capy_task *task)
{
    capy_err err = taskcheduler_init();

    if (err.code)
    {
        return err;
    }

    if (task->fd == -1)
    {
        return Ok;
    }

    if (epoll_ctl(task_scheduler->fd, EPOLL_CTL_DEL, task->fd, NULL) == -1)
    {
        if (err.code != ENOENT)
        {
            return err;
        }
    }

    return Ok;
}

static capy_err taskscheduler_subscribe(capy_task *task)
{
    capy_err err = taskcheduler_init();

    if (err.code)
    {
        return err;
    }

    struct epoll_event event = {
        .events = ((task->write) ? EPOLLOUT : EPOLLIN) | EPOLLET | EPOLLONESHOT,
        .data.ptr = task,
    };

    if (epoll_ctl(task_scheduler->fd, EPOLL_CTL_MOD, task->fd, &event) == -1)
    {
        capy_err err = ErrStd(errno);

        if (err.code != ENOENT)
        {
            return err;
        }

        if (epoll_ctl(task_scheduler->fd, EPOLL_CTL_ADD, task->fd, &event) == -1)
        {
            return ErrStd(errno);
        }
    }

    return Ok;
}

static capy_err taskscheduler_enqueue(capy_task *task)
{
    capy_err err = taskcheduler_init();

    if (err.code)
    {
        return err;
    }

    taskqueue_add(task_scheduler->queue, task);

    return Ok;
}

static capy_err taskscheduler_wait(void)
{
    capy_err err = taskcheduler_init();

    if (err.code)
    {
        return err;
    }

    taskscheduler_switch(task_scheduler->poller);

    if (task_scheduler->active == &task_scheduler->main && task_scheduler->cancel)
    {
        return ErrStd(ECANCELED);
    }

    return Ok;
}

static bool taskscheduler_canceled(void)
{
    if (task_scheduler == NULL)
    {
        return false;
    }

    return task_scheduler->cancel;
}

static capy_err taskscheduler_active(capy_task **task)
{
    capy_err err = taskcheduler_init();

    if (err.code)
    {
        return err;
    }

    *task = task_scheduler->active;

    return Ok;
}

static capy_err taskscheduler_join(Unused int timeout)
{
    if (task_scheduler == NULL)
    {
        return Ok;
    }

    if (task_scheduler->active != &task_scheduler->main)
    {
        return ErrStd(EINVAL);
    }

    while (task_scheduler->queue->size > 0)
    {
        taskscheduler_switch(task_scheduler->poller);
    }

    capy_arena_destroy(task_scheduler->arena);
    task_scheduler = NULL;

    return Ok;
}

// PUBLIC DEFINITIONS

size_t capy_thread_id(void)
{
    return pthread_self();
}
