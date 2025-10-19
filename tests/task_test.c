#include <capy/test.h>

#include "../src/capy.h"

static int test_taskqueue(void)
{
    capy_arena *arena = capy_arena_init(0, KiB(4));

    capy_taskqueue *queue = capy_taskqueue_init_(arena);

    capy_task tasks[] = {
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 10},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 9},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 8},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 7},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 6},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 5},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 4},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 3},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 2},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 1},
        {.queuepos = TASKQUEUE_REMOVED, .deadline.tv_sec = 0},
    };

    for (size_t i = 0; i < ArrLen(tasks); i++)
    {
        ExpectOk(capy_taskqueue_add_(queue, tasks + i));
    }

    ExpectEqPtr(capy_taskqueue_remove_(queue, tasks[0].queuepos), tasks + 0);
    ExpectEqU(tasks[0].queuepos, TASKQUEUE_REMOVED);

    for (size_t i = ArrLen(tasks); i > 1; i--)
    {
        capy_task *task = capy_taskqueue_pop_(queue);
        ExpectEqPtr(task, tasks + i - 1);
    }

    capy_arena_destroy(arena);
    return true;
}

static void test_task(testbench *t)
{
    runtest(t, test_taskqueue, "taskqueue");
}
