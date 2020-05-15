#include <common.h>
spinlock_t bigLock;

void kmt_init()
{
    spin_init(&bigLock, NULL);
}

int create(task_t* task, const char* name, void (*entry)(void* arg), void* arg)
{
    spin_lock(&bigLock);
    task->name = name;
    _Area stack = (_Area){&task->context + 1, task + 1};
   printf("%p %p %p\n", &task, &task->context + 1, task + 1);
    task->context = _kcontext(stack, entry, arg);
    // task->next = (TASKS_P + 1) % 32;
    task->next = 0;
    TASKS[TASKS_P] = task;
    printf("%p\n", TASKS[TASKS_P]);
    TASKS_P = (TASKS_P + 1) % 32;
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
       // printf("HERE\n");

    } while ((current - TASKS[0]) % _ncpu() != _cpu());

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