#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#    include <stdint.h>
typedef struct spinlock {
    intptr_t locked;
    int cpu;
    const char* name;
} spinlock_t;

void spin_init(spinlock_t* lk, const char* name);
void spin_lock(spinlock_t* lk);
void spin_unlock(spinlock_t* lk);

#endif