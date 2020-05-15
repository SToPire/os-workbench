#include<kmt.h>
#include<kernel.h>
int create(task_t* task, const char* name, void (*entry)(void* arg), void* arg)
{
    task->name = name;
    _Area stack = (_Area){&task->context + 1, task + 1};
    task->context = _kcontext(stack, entry, arg);
    task->next = (TASKS_P + 1) % 32;
    TASKS[TASKS_P] = task;
    TASKS_P = (TASKS_P + 1) % 32;
    //pmm->alloc(1);
    return 0;
}