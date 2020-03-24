#include <common.h>

static void os_init()
{
    pmm->init();
}
//static void* a[10];
static void os_run()
{
    for (const char* s = "Hello World from CPU #*\n"; *s; s++) {
        _putc(*s == '*' ? '0' + _cpu() : *s);
    }
    
    // printf("%p\n", a);
    //for (int i = 1; i <= 2000; i++) pmm->alloc(2048);
    //for (int i = 1; i <= 2000; i++) pmm->alloc(512);
    for (int i = 1; i <= 8000; i++) pmm->alloc(8);
    // for (int i = 1; i <= 2000; i++) pmm->alloc(4096);
    // for (int i = 1; i <= 2000; i++) pmm->alloc(1024);
    

    // if(_cpu()==0){
    //     a[0] = pmm->alloc(2048);
    //     a[1] = pmm->alloc(2048);
    //     for (volatile int i = 1; i <= 1000000; i++)        ;
    // }
    // if(_cpu()==1){
    //     pmm->free(a[0]);
    //     for (volatile int i = 1; i <= 1000000; i++)
    //         ;
    // }
    // if(_cpu()==0){
    //     pmm->alloc(2048);
    //     pmm->alloc(2048);
    // }

    // if (_cpu() == 1) {
    //     pmm->free(a[1]);
    //     pmm->free(a[2]);
    //     pmm->free(a[3]);
    //     pmm->free(a[4]);
    //     pmm->free(a[5]);
    //     for (volatile int i = 1; i <= 1000000; i++)
    //         ;
    // }
    // if (_cpu() == 0)
    //     for (volatile int i = 1; i <= 1000000; i++)
    //         ;
    // pmm->alloc(2048);
    // pmm->alloc(2048);
    // pmm->alloc(2048);

    while (1)
        ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
};
