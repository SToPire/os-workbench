#ifndef __SEM_H__
#define __SEM_H__

#define QSIZE 32
typedef struct semaphore {
    const char* name;
    int value;
    spinlock_t lock;

    int queue[QSIZE];
    int front, end;
} sem_t;

void sem_init(sem_t* sem, const char* name, int value);
void sem_wait(sem_t* sem);
void sem_signal(sem_t* sem);

#endif