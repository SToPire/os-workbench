#include <common.h>
#include<spinlock.h>

static void os_init()
{
    pmm->init();
   // kmt->init();
}
static void os_run()
{
    spinlock_t lk;
    if (_cpu() == 0){
        spin_lock(&lk);
        putstr("0 is holding the lock\n");
        spin_unlock(&lk);
    } else if (_cpu() == 1) {
        spin_lock(&lk);
        putstr("1 is holding the lock\n");
        spin_unlock(&lk);
    }
    while(1)
        ;
}

_Context* os_trap(_Event ev, _Context* context)
{
    return context;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
    .trap = os_trap,
};
