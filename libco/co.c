#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <inttypes.h>
#include <string.h>

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

struct co* colist[2];
int colistcnt;
struct co* co_start(const char* name, void (*func)(void*), void* arg)
{
    struct co* ptr = malloc(sizeof(struct co));
    strcpy(ptr->name, name);
    ptr->func = func;
    ptr->arg = arg;
    printf("112\n");

    ptr->status = CO_NEW;
    ptr->waiter = NULL;

    memset(ptr->stack, 0, sizeof(ptr->stack));
    colist[colistcnt++] = ptr;
    return ptr;
}

void co_wait(struct co* co)
{
    int val = setjmp(current->context);
    if (val == 0) {
        current = co;
        stack_switch_call(co->stack, co->func, (uintptr_t)co->arg);
    } else {
        free(co);
        return;
    }
}

void co_yield()
{
    int val = setjmp(current->context);
    if (val == 0) {
        int r = rand() % 2;
        if(colist[r]->status==CO_NEW)
            stack_switch_call(colist[r]->stack, colist[r]->func, (uintptr_t)colist[r]->arg);
        else
            longjmp(colist[r]->context, 1);
    } else {
        return;
    }
}
