#include <common.h>
spinlock_t bigSemLock;
void sem_init(sem_t* sem, const char* name, int value)
{
    spin_lock(&bigSemLock);
    sem->name = name;
    sem->value = value;
    spin_init(&sem->lock, NULL);
    memset(sem->queue, 0, sizeof(sem->queue));
    sem->front = sem->end = 0;
    printf("%d\n", sizeof(sem->queue));
    spin_unlock(&bigSemLock);
}

void sem_wait(sem_t* sem)
{
    spin_lock(&bigSemLock);
    spin_lock(&sem->lock);
    sem->value--;
    if (sem->value < 0) {
        current->status = SLEEPING;
    }
    spin_unlock(&bigSemLock);
}

void sem_signal(sem_t* sem)
{
}