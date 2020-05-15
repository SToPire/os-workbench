#include<kmt.h>
#include<kernel.h>
#include<spinlock.h>
spinlock_t bigLock;

void kmt_init()
{
    spin_init(&bigLock,NULL);
}

int create(task_t* task, const char* name, void (*entry)(void* arg), void* arg)
{
    spin_lock(&bigLock);
    task->name = name;
    _Area stack = (_Area){&task->context + 1, task + 1};
    task->context = _kcontext(stack, entry, arg);
    task->next = (TASKS_P + 1) % 32;
    TASKS[TASKS_P] = task;
    TASKS_P = (TASKS_P + 1) % 32;
    spin_unlock(&bigLock);

    return 0;
}

MODULE_DEF(kmt) = {
    .init = kmt_init,
    .create = create,
    .teardown = NULL,
};