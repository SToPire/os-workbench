#include <common.h>

static void os_init()
{
    pmm->init();
}

static void os_run()
{
    for (const char* s = "Hello World from CPU #*\n"; *s; s++) {
        _putc(*s == '*' ? '0' + _cpu() : *s);
    }
    for (int i = 1; i <= 4; i++) {
        pmm->alloc(2048);
        //pmm->free(a);
    }
    while (1)
        ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
};
