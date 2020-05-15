#include<kmt.h>

int create(task_t* task, const char* name, void (*entry)(void* arg), void* arg)
{
    task->name = name;
    return 0;
}