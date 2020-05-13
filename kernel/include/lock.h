#include <stdint.h>
// intptr_t atomic_xchg(volatile intptr_t* addr,
//                      intptr_t newval)
// {
//     // swap(*addr, newval);
//     intptr_t result;
//     asm volatile("lock xchg %0, %1"
//                  : "+m"(*addr), "=a"(result)
//                  : "1"(newval)
//                  : "cc");
//     return result;
// }
typedef struct spinlock {
    intptr_t locked;
    int cpu;
} spinlock_t;

void spin_init(spinlock_t* lk)
{
    lk->locked = 0;
    lk->cpu = 0;
}

void spin_lock(spinlock_t* lk)
{
    while (_atomic_xchg(&lk->locked, 1))
        ;
}

void spin_unlock(spinlock_t* lk)
{
    _atomic_xchg(&lk->locked, 0);
}