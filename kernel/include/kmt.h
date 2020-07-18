#ifndef __KMT_H__
#define __KMT_H__

#include <am.h>
#include<vfs.h>
typedef struct task task_t;

enum {
    INVALID,
    READY,
    SLEEPING,
    RUNNING,
};

struct task {
    const char* name;
    int next;
    int num;
    int status;

    _Context* context;
    _Area stack;

    int fds[128];
    char cwd[128];
    //int sticky;
};
#define MAX_TASKS 32
task_t* TASKS[MAX_TASKS];
int TASKS_FREE;
int TASKS_HEAD;
int TASKS_CNT;
int TASKS_LAST_CREATE;

int create(task_t* task, const char* name, void (*entry)(void* arg), void* arg);
void teardown(task_t* task);
_Context* scheduler(_Event ev, _Context* _Context);

#define MAX_INTR 31
struct _INTR{
    int valid;
    handler_t handler;
    int event;
} INTR[32];

spinlock_t trapLock;
spinlock_t bigKmtLock;

struct cpu_local {
    task_t* current;
    //task_t* sticky;
} cpu_local[8];

#endif