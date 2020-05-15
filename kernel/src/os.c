#include <common.h>
spinlock_t lk;

void th1()
{
    spin_lock(&lk);
    _putc('A');
    spin_unlock(&lk);
}
void th2()
{
    spin_lock(&lk);
    _putc('B');
    spin_unlock(&lk);
}

static void os_init()
{
    pmm->init();
    kmt->init();
    spin_init(&lk, NULL);  //for test

    //task_t* tt = pmm->alloc(sizeof(task_t));
    task_t tt;
    kmt->create(&tt, "th1", th1, NULL);
    //kmt->create(pmm->alloc(sizeof(task_t)), "th2", th2, NULL);
}

static void os_run()
{
    printf("\nhere\n");
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
    return scheduler(ev,context);
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
