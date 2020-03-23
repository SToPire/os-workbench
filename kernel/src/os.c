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
    //void* a[200000];
    for (int i = 0; i < 20; i++) {
        pmm->alloc(1);
        //a[i] = pmm->alloc(1);
        //pmm->free(a[i]);
    }
        while (1)
            ;
    }

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
};
