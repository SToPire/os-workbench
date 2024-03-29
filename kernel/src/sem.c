#include <common.h>
spinlock_t bigSemLock;
#define current cpu_local[_cpu()].current
void sem_init(sem_t* sem, const char* name, int value)
{
   // spin_lock(&bigSemLock);

    sem->name = name;
    sem->value = value;
    spin_init(&sem->lock, sem->name);
    memset(sem->queue, 0, sizeof(sem->queue));
    sem->front = sem->end = 0;

    //spin_unlock(&bigSemLock);
}

void sem_wait(sem_t* sem)
{
    //spin_lock(&bigSemLock);
    //printf("P:%s\n", sem->name);
    spin_lock(&sem->lock);
    bool flag = true;
    sem->value--;
    if (sem->value < 0) {
        current->status = SLEEPING;
        sem->queue[sem->front] = current->num;
        sem->front = (sem->front + 1) % QSIZE;
        flag = false;
        //printf("%d %d %d\n", sem->front, sem->end,sem->queue[0]);
    }
    spin_unlock(&sem->lock);
    //printf("P-value:%d\n", sem->value);
    if (flag == false) {
       // spin_unlock(&bigSemLock);
        //printf("p0:%d p1:%d\n",TASKS[0]->status,TASKS[1]->status);
        _yield();
    }

    //if (flag == true) spin_unlock(&bigSemLock);
}

void sem_signal(sem_t* sem)
{
    //spin_lock(&bigSemLock);
    //printf("V:%s\n", sem->name);

    spin_lock(&sem->lock);
    sem->value++;
    if (sem->front != sem->end) {
        TASKS[sem->queue[sem->end]]->status = READY;

        sem->end = (sem->end + 1) % QSIZE;
        //printf("%d %d\n", sem->front, sem->end);
    }
    //printf("V-value:%d\n", sem->value);

    spin_unlock(&sem->lock);

    //spin_unlock(&bigSemLock);
}