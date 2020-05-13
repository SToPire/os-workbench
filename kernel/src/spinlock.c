#include <common.h>
#include <spinlock.h>

void spin_init(spinlock_t* lk)
{
    lk->locked = 0;
    //lk->cpu = 0;
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