#include <common.h>
spinlock_t lk;

void th1()
{
    //assert(_cpu() == 0);
    while (1) {
        spin_lock(&lk);
        assert(_intr_read() == 0);
        //printf("This is th1 running in CPU %d!\n",_cpu());
        spin_unlock(&lk);
        for (volatile int i = 1; i < 100000; i++)
            ;
    }
}
void th2()
{
    //assert(_cpu() == 1);
    while (1) {
        spin_lock(&lk);
        assert(_intr_read() == 0);
        //printf("This is th2 running in CPU %d!\n", _cpu());
        spin_unlock(&lk);
        for (volatile int i = 1; i < 100000; i++)
            ;
    }
}
void th3()
{
    //assert(_cpu() == 0);
    while (1) {
        spin_lock(&lk);
        assert(_intr_read() == 0);
        //printf("This is th3 running in CPU %d!\n", _cpu());
        spin_unlock(&lk);
        for (volatile int i = 1; i < 100000; i++)
            ;
    }
}
static void os_init()
{
    pmm->init();
    kmt->init();
    spin_init(&lk, NULL);  //for test

    task_t* t1 = pmm->alloc(sizeof(task_t));
    task_t* t2 = pmm->alloc(sizeof(task_t));
    task_t* t3 = pmm->alloc(sizeof(task_t));

    kmt->create(t1, "th1", th1, NULL);
    kmt->create(t2, "th2", th2, NULL);
    kmt->create(t3, "th3", th3, NULL);

    kmt->teardown(t1);
}

static void os_run()
{
    _intr_write(1);
    // if (_cpu() == 0) {
    //     spin_lock(&lk);
    //     putstr("0 is holding the lock\n");
    //     spin_unlock(&lk);
    // } else if (_cpu() == 1) {
    //     spin_lock(&lk);
    //     putstr("1 is holding the lock\n");
    //     spin_unlock(&lk);
    // }

    _yield();
}

_Context* os_trap(_Event ev, _Context* context)
{
    //return context;
    return scheduler(ev, context);
}
void on_irq(int seq, int event, handler_t handler)
{
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
    .trap = os_trap,
    .on_irq = on_irq,
};
