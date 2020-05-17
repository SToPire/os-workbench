#include<am.h>
typedef struct task task_t;

struct task {
    union {
        struct {
            const char* name;
            int next;
            int num;
            _Context* context;
        };
        uint8_t stack[4096];
    };
};

#define MAX_TASKS 32
task_t* TASKS[MAX_TASKS];
int TASKS_P;
int TASKS_FREE;
int TASKS_HEAD;
int TASKS_CNT;

int create(task_t* task, const char* name, void (*entry)(void* arg), void* arg);
void teardown(task_t* task);