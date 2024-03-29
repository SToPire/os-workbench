#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define STACK_SIZE 64 * 1024
#define CO_SIZE 128

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

void* x86_64_addr_align(void * addr) // alignment is required before 'call' instrucion
{
#if __x86_64__
    return (void*)(((uintptr_t)addr >> 4 << 4)- 8);
#else
    return addr;
#endif
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
    int num;

    enum co_status status;      // 协程的状态
    struct co* waiter;          // 是否有其他协程在等待当前协程
    jmp_buf context;            // 寄存器现场 (setjmp.h)
    uint8_t stack[STACK_SIZE];  // 协程的堆栈
};

struct co* colist[CO_SIZE];

struct freeListNode{
    int num;
    int next;
    int alloc;
} freelist[STACK_SIZE + 1], *head;

void Free(struct co* co)
{
    freelist[co->num].next = head->num;
    freelist[co->num].alloc = 0;
    head = &freelist[co->num];
    free(co);
}

__attribute__((constructor)) void
co_init()
{
    srand(time(0));
    colist[0] = malloc(sizeof(struct co));
    colist[0]->status = CO_RUNNING;
    colist[0]->num = 0;
    current = colist[0];

    freelist[0].next = freelist[0].num = 0;
    freelist[0].alloc = 1;
    for (int i = 1; i < STACK_SIZE; ++i) {
        freelist[i].num = i;
        freelist[i].next = i + 1;
        freelist[i].alloc = 0;
    }
    freelist[STACK_SIZE].num = freelist[STACK_SIZE].next = 0;
    head = &freelist[1];
}
struct co* co_start(const char* name, void (*func)(void*), void* arg)
{
    struct co* ptr = malloc(sizeof(struct co));

    ptr->name = malloc(sizeof(name));
    strcpy(ptr->name, name);
    ptr->func = func;
    ptr->arg = arg;

    ptr->status = CO_NEW;
    ptr->waiter = NULL;
    ptr->num = head->num;

    memset(ptr->stack, 0, sizeof(ptr->stack));
    colist[head->num] = ptr;
    freelist[head->num].alloc = 1;
    head = &freelist[head->next];
    return ptr;
}

void co_wait(struct co* co)
{
    if (co->status == CO_DEAD) {
        Free(co);
        return;
    }
    co->waiter = current;
    current->status = CO_WAITING;
    while (co->status != CO_DEAD)
        co_yield();
    Free(co);
}

void wrapper(int num)
{
    colist[num]->status = CO_RUNNING;
    (colist[num]->func)(colist[num]->arg);
    colist[num]->status = CO_DEAD;
    if (colist[num]->waiter) {
        colist[num]->waiter->status = CO_RUNNING;
    }
    co_yield();
}

void co_yield()
{
    int val = setjmp(current->context);
    if (val == 0) {
        int r = rand() % CO_SIZE;
        while (freelist[r].alloc == 0 || colist[r]->status == CO_WAITING || colist[r]->status == CO_DEAD)
            r = rand() % CO_SIZE;
        current = colist[r];
        if (current->status == CO_NEW) {
            current->status = CO_RUNNING;
            stack_switch_call(x86_64_addr_align(current->stack + STACK_SIZE), wrapper, r);
        } else {
            longjmp(current->context, 1);
        }
    } else {
        return;
    }
}
