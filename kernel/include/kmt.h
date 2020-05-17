#include<am.h>
typedef struct task task_t;

enum {
    INVALID, READY, SLEEPING,
};
struct task {
    union {
        struct {
            const char* name;
            int next;
            int num;
            int status;
            _Context* context;
        };
        uint8_t stack[4096];
    };
};

#define MAX_TASKS 32
task_t* TASKS[MAX_TASKS];
int TASKS_FREE;
int TASKS_HEAD;
int TASKS_CNT;
int TASKS_LAST_CREATE;

int create(task_t* task, const char* name, void (*entry)(void* arg), void* arg);
void teardown(task_t* task);