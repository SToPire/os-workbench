#include <common.h>
spinlock_t bigLock;

void kmt_init()
{
    spin_init(&bigLock, NULL);
    for (int i = 0; i < 32;i++){
        TASKS[i]->next = (i + 1) % 32;
    }
}

int create(task_t* task, const char* name, void (*entry)(void* arg), void* arg)
{
    spin_lock(&bigLock);
    task->name = name;
    _Area stack = (_Area){&task->context + 1, task + 1};
    //printf("%p %p %p\n", task, &task->context + 1, task + 1);
    task->context = _kcontext(stack, entry, arg);

    if (MAX_TASKS == TASKS_CNT) panic("No more TASKS can be created!");
    if (TASKS_CNT++ == 0)
        TASKS_HEAD = TASKS_FREE;

    task->next = TASKS_HEAD;
    int nxt = TASKS[TASKS_FREE]->next;
    TASKS[TASKS_FREE] = task;
    TASKS_FREE = nxt;
    printf("%d %d %d \n", TASKS_FREE, TASKS_HEAD, TASKS_CNT);
    spin_unlock(&bigLock);

    return 0;
}

struct cpu_local {
    task_t* current;
} cpu_local[8];
#define current cpu_local[_cpu()].current

_Context* scheduler(_Event ev, _Context* _Context)
{
    if (!current) {
        current = TASKS[0];
    } else {
        current->context = _Context;
    }
    do {
        current = TASKS[current->next];
    } while ((current->num) % _ncpu() != _cpu());

    return current->context;
}

MODULE_DEF(kmt) = {
    .init = kmt_init,
    .create = create,
    .teardown = NULL,
    .spin_init = spin_init,
    .spin_lock = spin_lock,
    .spin_unlock = spin_unlock,
    .sem_init = NULL,
    .sem_wait = NULL,
    .sem_signal = NULL,
};