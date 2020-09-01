#include "threads.h"

// Allowed libraries:
// * pthread_mutex_lock, pthread_mutex_unlock
// * pthread_cond_wait, pthread_cond_signal, pthread_cond_broadcast
// * sem_init, sem_wait, sem_post
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
volatile int putting = 0;
volatile int cx = 0, cy = 0, cz = 0;
void fish_init()
{
    // TODO
}

void fish_before(char ch)
{
    pthread_mutex_lock(&lock);
    if(ch=='X' || ch == 'Y'){
        while (cx + cy >= cz - 1 || putting) pthread_cond_wait(&cond, &lock);
    }else{
        while (cz >= cx + cy + 10 || putting) pthread_cond_wait(&cond, &lock);
    }
    putting = 1;
    pthread_mutex_unlock(&lock);
}

void fish_after(char ch)
{
    pthread_mutex_lock(&lock);
    switch(ch){
        case 'X': ++cx; break;
        case 'Y': ++cy; break;
        case 'Z': ++cz; break;
    }
    putting = 0;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&lock);
}

static const char roles[] = "XXXXXYYYYZZZ";

void fish_thread(int id)
{
    char role = roles[id];
    while (1) {
        fish_before(role);
        putchar(role);  // should not hold *any* mutex lock now
        fish_after(role);
    }
}

int main()
{
    setbuf(stdout, NULL);
    fish_init();
    for (int i = 0; i < strlen(roles); i++)
        create(fish_thread);
    join(NULL);
}
