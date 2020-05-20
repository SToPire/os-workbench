typedef struct semaphore{
    const char* name;
    int value;
    spinlock_t lock;

    int queue[32];
    int front, end;
} sem_t;

void sem_init(sem_t* sem, const char* name, int value);
void sem_wait(sem_t* sem);
void sem_signal(sem_t* sem);