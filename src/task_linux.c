#include <capy/macros.h>
#include <pthread.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>

#include "capy_linux.h"

#define SIGNAL_EPOLL_EVENT -1ULL

// INTERNAL DEFINITIONS

void capy_poll_(void *data)
{
    capy_scheduler *scheduler = data;

    capy_task *ready[32];
    int ready_count = 0;
    int ready_max = ArrLen(ready);
    int timeout_max = ready_max / 2;
    struct epoll_event events[ready_max];

    for (;;)
    {
        ready_count = 0;

        int timeout = Seconds(10);

        while (scheduler->queue->size > 0 && ready_count < timeout_max)
        {
            int64_t ms = capy_taskqueue_timeout_(scheduler->queue);

            if (ms <= 0)
            {
                capy_task *task = capy_taskqueue_pop_(scheduler->queue);
                capy_poll_remove_(scheduler, task);
                ready[ready_count++] = task;
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

                continue;
            }

            for (int i = 0; i < count; i++)
            {
                if (events[i].data.u64 == SIGNAL_EPOLL_EVENT)
                {
                    capy_task *task = capy_taskqueue_remove_(scheduler->queue, scheduler->main->queuepos);
                    capy_poll_remove_(scheduler, task);
                    scheduler->cancel = true;
                }
                else
                {
                    capy_task *task = events[i].data.ptr;
                    capy_taskqueue_remove_(scheduler->queue, task->queuepos);
                    ready[ready_count++] = task;
                }
            }
        }

        for (int i = 0; i < ready_count; i++)
        {
            capy_scheduler_switch_(scheduler, ready[i]);
        }

        capy_scheduler_switch_(scheduler, scheduler->main);
    }
}

capy_err capy_poll_init_(capy_scheduler *scheduler)
{
    capy_poll *poll = Make(scheduler->arena, capy_poll, 1);

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
        .data.u64 = SIGNAL_EPOLL_EVENT,
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

capy_err capy_poll_destroy_(capy_scheduler *scheduler)
{
    close(scheduler->poll->fd);
    close(scheduler->poll->signal_fd);
    return Ok;
}

capy_err capy_poll_remove_(capy_scheduler *scheduler, capy_task *task)
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

capy_err capy_poll_add_(capy_scheduler *scheduler, capy_task *task)
{
    if (task->fd != -1)
    {
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
    }

    return Ok;
}

// PUBLIC DEFINITIONS

size_t capy_thread_id(void)
{
    return pthread_self();
}
