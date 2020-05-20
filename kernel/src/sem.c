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

    spin_unlock(&bigSemLock);
}

void sem_wait(sem_t* sem)
{
    spin_lock(&bigSemLock);
    printf("P:%s\n", sem->name);
    spin_lock(&sem->lock);
    bool flag = true;
    sem->value--;
    if (sem->value < 0) {
        current->status = SLEEPING;
        sem->queue[sem->front] = current->num;
        sem->front = (sem->front + 1) % QSIZE;
        flag = false;
    }
    printf("here?\n");
    spin_unlock(&sem->lock);
    if(flag == false){
        spin_unlock(&bigSemLock);
        _yield();
    }

    spin_unlock(&bigSemLock);
}

void sem_signal(sem_t* sem)
{
    spin_lock(&bigSemLock);
    printf("V:%s\n", sem->name);

    spin_lock(&sem->lock);
    sem->value++;
    if(sem->front != sem->end){
        TASKS[sem->end]->status = READY;
        sem->end = (sem->end + 1) % QSIZE;
    }
    spin_unlock(&sem->lock);

    spin_unlock(&bigSemLock);
}