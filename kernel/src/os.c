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
    void* a[200000];
    printf("%p\n", a);
    for (int i = 0; i < 16000; i+=2) {
         a[i+_cpu()] = pmm->alloc(4096);
    }
    for (int i = 15998; i >= 0; i -= 2) {
        pmm->free(a[i + _cpu()]);
    }
    // for (int i = 0; i < 16000; i += 2) {
    //     a[i + _cpu()] = pmm->alloc(1024);
    // }
    // for (int i = 15998; i >= 0; i -= 2) {
    //     pmm->free(a[i + _cpu()]);
    // }
    // for (int i = 0; i < 16000; i += 2) {
    //     a[i + _cpu()] = pmm->alloc(4096);
    // }
    // for (int i = 0; i < 16000; i += 2) {
    //     pmm->free(a[i + _cpu()]);
    // }
   // pmm->alloc(4096);
    // for (int i = 0; i < 8000; i++) {
    //     pmm->free(a[i]);
    // }
    while (1)
        ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
};
