#include <common.h>

static void os_init()
{
    pmm->init();
   // kmt->init();
}
static void os_run()
{
    _intr_write(1);
    while (1){
        if (_cpu() == 0) _putc('0');
        if (_cpu() == 1) _putc('1');
    }
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
