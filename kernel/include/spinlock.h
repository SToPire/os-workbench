#include <stdint.h>
typedef struct spinlock {
    intptr_t locked;
    int cpu;
} spinlock_t;

void spin_init(spinlock_t* lk);
void spin_lock(spinlock_t* lk);
void spin_unlock(spinlock_t* lk);