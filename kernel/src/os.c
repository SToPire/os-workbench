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
    for (int i = 1; i <= 5;i++){
        a[i + 5 * _cpu()] = pmm->alloc(4096);
        printf("cpu:%d a[%d]:%x\n", _cpu(), i + 5 * _cpu(), a[i + 5 * _cpu()]);
    }

    if (_cpu() == 1) pmm->free(a[5]);
   // pmm->alloc(4096);
   // pmm->alloc(4096);

    while (1)
        ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
};
