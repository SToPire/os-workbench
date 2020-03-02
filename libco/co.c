#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define STACK_SIZE 64 * 1024

static inline void stack_switch_call(void* sp, void* entry, uintptr_t arg)
{
    asm volatile(
#if __x86_64__
        "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
        :
        : "b"((uintptr_t)sp), "d"(entry), "a"(arg)
#else
        "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
        :
        : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
#endif
    );
}

struct co* current;

enum co_status {
    CO_NEW = 1,  // 新创建，还未执行过
    CO_RUNNING,  // 已经执行过
    CO_WAITING,  // 在 co_wait 上等待
    CO_DEAD,     // 已经结束，但还未释放资源
};

struct co {
    char* name;
    void (*func)(void*);  // co_start 指定的入口地址和参数
    void* arg;

    enum co_status status;      // 协程的状态
    struct co* waiter;          // 是否有其他协程在等待当前协程
    jmp_buf context;            // 寄存器现场 (setjmp.h)
    uint8_t stack[STACK_SIZE];  // 协程的堆栈
};

struct co* colist[3];
int colistcnt;

__attribute__((constructor)) void co_init()
{
    srand(time(0));
    colistcnt = 1;
    printf("here\n");
    colist[0]->status = CO_RUNNING;
    current = colist[0];
}
struct co* co_start(const char* name, void (*func)(void*), void* arg)
{
    struct co* ptr = malloc(sizeof(struct co));
    //strcpy(ptr->name, name);
    ptr->func = func;
    ptr->arg = arg;

    ptr->status = CO_NEW;
    ptr->waiter = NULL;

    memset(ptr->stack, 0, sizeof(ptr->stack));
    colist[colistcnt++] = ptr;
    return ptr;
}

void co_wait(struct co* co)
{
    co->waiter = current;
    current->status = CO_WAITING;
    while(co->status!=CO_DEAD)
        co_yield();
    free(co);
}

void wrapper(int num)
{
    colist[num]->status = CO_RUNNING;
    (colist[num]->func)(colist[num]->arg);
    colist[num]->status = CO_DEAD;
    if(colist[num]->waiter){
        colist[num]->waiter->status = CO_RUNNING;
    }
    co_yield();
}

void co_yield()
{
    int val = setjmp(current->context);
    if (val == 0) {
        int r = rand() % 3;
        while (colist[r]->status == CO_WAITING && colist[r]->status == CO_DEAD)
            r = rand() % 3;
        current = colist[r];
        if (current->status == CO_NEW) {
            current->status = CO_RUNNING;
            stack_switch_call(current->stack + STACK_SIZE - 8, wrapper, r);
        }else {
            longjmp(current->context, 1);
        }
    } else {
        return;
    }
}
