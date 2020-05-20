#include<common.h>
spinlock_t bigSemLock;
void sem_init(sem_t* sem, const char* name, int value)
{
    spin_lock(&bigSemLock);
    sem->name = name;
    sem->value = value;
    spin_init(&sem->lock,NULL);
    spin_unlock(&bigSemLock);
}

void sem_wait(sem_t* sem)
{
    spin_lock(&bigSemLock);
    

    
    spin_unlock(&bigSemLock);
}

void sem_signal(sem_t* sem)
{

}