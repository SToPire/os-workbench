#include <common.h>

struct _cpustat {
    int ncli;
    int intena;
} cpustat[8];

void pushcli();
void popcli();
int holding(spinlock_t* lk);

void spin_init(spinlock_t* lk, const char* name)
{
    lk->locked = 0;
    lk->cpu = 0;
    //lk->name = name;
}

void spin_lock(spinlock_t* lk)
{
    pushcli();
    //panic_on(holding(lk), "Trying to hold a lock which already held by this cpu");
    if (holding(lk)) {
        printf("Tring to relock:cpu%d\n", _cpu());
       _halt(1);
    }
    while (_atomic_xchg(&lk->locked, 1))
        ;
    lk->cpu = _cpu();

    printf("spin_lock from cpu %d\n", _cpu());
}

void spin_unlock(spinlock_t* lk)
{
    panic_on(!holding(lk), "releasing a unheld lock");
    lk->cpu = 0;
    printf("spin_unlock from cpu %d\n", _cpu());
    _atomic_xchg(&lk->locked, 0);
    popcli();
}

int holding(spinlock_t* lk)
{
    int r;
    pushcli();
    r = lk->locked && lk->cpu == _cpu();
    popcli();
    return r;
}

void pushcli()
{
    int i = _intr_read();
    _intr_write(0);
    if (cpustat[_cpu()].ncli == 0)
        cpustat[_cpu()].intena = i;
    ++cpustat[_cpu()].ncli;
}

void popcli()
{
    panic_on(_intr_read() == 1, "interrupt is open while lock is held");
    panic_on((--cpustat[_cpu()].ncli) < 0, "ncli < 0");
    if (cpustat[_cpu()].ncli == 0 && cpustat[_cpu()].intena)
        _intr_write(1);
}
