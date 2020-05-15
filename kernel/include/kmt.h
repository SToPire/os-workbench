#include<am.h>
typedef struct task task_t;

struct task {
    union {
        struct {
            const char* name;
            int next;
            _Context* context;
        };
        uint8_t stack[4096];
    };
};

task_t* TASKS[32];
int TASKS_P;

int create(task_t* task, const char* name, void (*entry)(void* arg), void* arg);
void teardown(task_t* task);