#include<kmt.h>

int create(task_t* task, const char* name, void (*entry)(void* arg), void* arg)
{
    task->name = name;
    _Area stack = (_Area){&task->context + 1, task + 1};
    task->context = _kcontext(stack, entry, arg);
    return 0;
}