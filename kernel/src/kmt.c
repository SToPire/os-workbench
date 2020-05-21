#include <common.h>
#define STACK_SIZE 4096

int TASKS_PTR;
void kmt_init()
{
    kmt->spin_init(&bigKmtLock, NULL);
    memset(INTR, 0, sizeof(INTR));
    os->on_irq(MAX_INTR, _EVENT_NULL, scheduler);
}

int create(task_t* task, const char* name, void (*entry)(void* arg), void* arg)
{
    kmt->spin_lock(&bigKmtLock);
    task->name = name;
    task->status = READY;
    //_Area stack = (_Area){&task->context + 1, task + 1};
    task->stack.start = pmm->alloc(STACK_SIZE);
    task->stack.end = task->stack.start + STACK_SIZE;
    task->context = _kcontext(task->stack, entry, arg);

    if (MAX_TASKS == TASKS_CNT) panic("No more TASKS can be created!");
    if (TASKS_CNT++ == 0)  // first task in os
        TASKS_HEAD = TASKS_FREE;
    else
        TASKS[TASKS_LAST_CREATE]->next = TASKS_FREE;
    task->num = TASKS_FREE;
    task->next = TASKS_HEAD;
    TASKS[TASKS_FREE] = task;
    TASKS_LAST_CREATE = TASKS_FREE;

    for (int i = 1; i <= MAX_TASKS; i++) {  // update TASKS_FREE
        if (!TASKS[(TASKS_FREE + i) % MAX_TASKS] || TASKS[(TASKS_FREE + i) % MAX_TASKS]->status == INVALID) {
            TASKS_FREE = (TASKS_FREE + i) % MAX_TASKS;
            break;
        }
    }

    TASKS_PTR = TASKS_HEAD;

    kmt->spin_unlock(&bigKmtLock);

    return 0;
}

void teardown(task_t* task)
{
    kmt->spin_lock(&bigKmtLock);
    int tmp;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (TASKS[i]->next == task->num) {
            tmp = TASKS[i]->num;
            break;
        }
        if (i == MAX_TASKS - 1) panic("Illegal teardown");
    }
    TASKS[tmp]->next = task->next;
    if (TASKS_LAST_CREATE == task->num) TASKS_LAST_CREATE = tmp;
    if (TASKS_HEAD == task->num) TASKS_HEAD = task->next;

    tmp = task->num;
    pmm->free(task->stack.start);
    memset(TASKS[tmp], 0, sizeof(task_t));
    --TASKS_CNT;
    kmt->spin_unlock(&bigKmtLock);
}

// _Context* scheduler(_Event ev, _Context* _Context)
// {
//     if (!current) {
//         current = TASKS[TASKS_HEAD];
//     } else {
//         current->context = _Context;
//     }
//     do {
//         current = TASKS[current->next];
//     } while ((current->num) % _ncpu() != _cpu() || TASKS[current->num]->status != READY);

//     return current->context;
// }

_Context* scheduler(_Event ev, _Context* _Context)
{
    if (cpu_local[_cpu()].sticky != NULL) {
        cpu_local[_cpu()].sticky->sticky = 0;
        printf("first if        %d %d %d\n", cpu_local[_cpu()].sticky->num, cpu_local[_cpu()].sticky->status, cpu_local[_cpu()].sticky->sticky);
        cpu_local[_cpu()].sticky = NULL;
    }
    task_t* i = TASKS[TASKS_PTR];
    printf("%d %d ptr:%d\n", TASKS[0]->status,TASKS[0]->sticky,TASKS_PTR);

    for (int j = 0; j < MAX_TASKS; j++, i = TASKS[i->next]) {
        printf("%d %d %d\n", i->num, i->status, i->sticky);
        if (i->status == READY && i->sticky == 0) {
            TASKS_PTR = TASKS[i->next]->num;
            break;
        }
        assert(j <= 20);
    }
    if (current) {
        printf("second if\n");
        current->status = READY;
        current->context = _Context;
        current->sticky = 1;
        cpu_local[_cpu()].sticky = current;
    }
    current = i;
    current->status = RUNNING;
    printf("i:%d\n", i->num);
    printf("%d %d %d\n", TASKS[0]->sticky, TASKS[1]->sticky, TASKS[2]->sticky);
    return current->context;
}

MODULE_DEF(kmt) = {
    .init = kmt_init,
    .create = create,
    .teardown = teardown,
    .spin_init = spin_init,
    .spin_lock = spin_lock,
    .spin_unlock = spin_unlock,
    .sem_init = sem_init,
    .sem_wait = sem_wait,
    .sem_signal = sem_signal,
};