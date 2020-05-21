#include <common.h>
spinlock_t trapLock;
// sem_t empty, fill;
// #define P kmt->sem_wait
// #define V kmt->sem_signal
// void producer(void* arg)
// {
//     while (1) {
//         P(&empty);
//         _putc('(');
//         V(&fill);
//     }
// }
// void consumer(void* arg)
// {
//     while (1) {
//         P(&fill);
//         _putc(')');
//         V(&empty);
//     }
// }

// void th1()
// {
//     //assert(_cpu() == 0);
//     while (1) {
//         spin_lock(&lk);
//         assert(_intr_read() == 0);
//         printf("This is th1 running in CPU %d!\n",_cpu());
//         spin_unlock(&lk);
//         for (volatile int i = 1; i < 100000; i++)
//             ;
//     }
// }
// void th2()
// {
//     //assert(_cpu() == 1);
//     while (1) {
//         spin_lock(&lk);
//         assert(_intr_read() == 0);
//         printf("This is th2 running in CPU %d!\n", _cpu());
//         spin_unlock(&lk);
//         for (volatile int i = 1; i < 100000; i++)
//             ;
//     }
// }
// void th3()
// {
//     //assert(_cpu() == 0);
//     while (1) {
//         spin_lock(&lk);
//         assert(_intr_read() == 0);
//         printf("This is th3 running in CPU %d!\n", _cpu());
//         spin_unlock(&lk);
//         for (volatile int i = 1; i < 100000; i++)
//             ;
//     }
// }
static void os_init()
{
    pmm->init();
    kmt->init();

    spin_init(&trapLock, NULL);  
    //sem_init(&sema, NULL, 0);

    // task_t* t1 = pmm->alloc(sizeof(task_t));
    // task_t* t2 = pmm->alloc(sizeof(task_t));
    // task_t* t3 = pmm->alloc(sizeof(task_t));

    // kmt->create(t1, "th1", th1, NULL);
    // kmt->create(t2, "th2", th2, NULL);
    // kmt->create(t3, "th3", th3, NULL);

    // kmt->sem_init(&empty, "empty", 5);  // 缓冲区大小为 5
    // kmt->sem_init(&fill, "fill", 0);
    // for (int i = 0; i < 4; i++)  // 4 个生产者
    //     kmt->create(pmm->alloc(sizeof(task_t)), "producer", producer, NULL);
    // for (int i = 0; i < 5; i++)  // 5 个消费者
    //     kmt->create(pmm->alloc(sizeof(task_t)), "consumer", consumer, NULL);
}

static void os_run()
{
    _intr_write(1);

    _yield();
}

_Context* os_trap(_Event ev, _Context* context)
{
    //spin_lock(&trapLock);
    _Context* next = NULL;
    for (int i = 0; i <= MAX_INTR;i++) {
        if (INTR[i].valid == 1 && (INTR[i].event == _EVENT_NULL || INTR[i].event == ev.event)) {
            printf("here:%d\n", i);
            _Context* r = INTR[i].handler(ev, context);
            panic_on(r && next, "returning multiple contexts");
            if (r) next = r;
        }
    }
    panic_on(!next, "returning NULL context");
    //spin_unlock(&trapLock);
    return next;
}
void on_irq(int seq, int event, handler_t handler)
{
    assert(seq >= 0 && seq <= MAX_INTR);
    INTR[seq].valid = 1;
    INTR[seq].event = event;
    INTR[seq].handler = handler;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
    .trap = os_trap,
    .on_irq = on_irq,
};
