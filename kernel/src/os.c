#include <common.h>
//spinlock_t lk;

static void os_init()
{
    pmm->init();
    kmt->init();
    //spin_init(&lk, NULL);  //for test
}
static void os_run()
{
    _intr_write(0);
    // if (_cpu() == 0) {
    //     spin_lock(&lk);
    //     putstr("0 is holding the lock\n");
    //     spin_unlock(&lk);
    // } else if (_cpu() == 1) {
    //     spin_lock(&lk);
    //     putstr("1 is holding the lock\n");
    //     spin_unlock(&lk);
    // }



    while(1)
        ;
}

_Context* os_trap(_Event ev, _Context* context)
{
    return context;
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
