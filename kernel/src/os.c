#include <common.h>

static void os_init()
{
    pmm->init();
   // kmt->init();
}
static void os_run()
{
    _intr_write(1);
    //printf("%d\n", _cpu());
    while (1)
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
