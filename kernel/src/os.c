#include <common.h>

static void os_init()
{
    pmm->init();
}

static void os_run()
{
    // for (const char* s = "Hello World from CPU #*\n"; *s; s++) {
    //     _putc(*s == '*' ? '0' + _cpu() : *s);
    // }
    void* a[10000];
    for (int i = 0; i < 10000; i++) {
        a[i] = pmm->alloc(4096);
    }
    for (int i = 0; i < 10000; i++) {
        pmm->free(a[i]);
    }
        while (1)
            ;
    }

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
};
