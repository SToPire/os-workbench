#include <common.h>
#include <devices.h>
#include <user.h>
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

// spinlock_t lk;
// void th(void* s)
// {
//     while (1) {
//         spin_lock(&lk);
//         assert(_intr_read() == 0);
//         //printf("This is %s running in CPU %d!\n",(const char*)s,_cpu());
//         spin_unlock(&lk);
//         for (volatile int i = 1; i < 100000; i++)
//             ;
//     }
// }
// void th4(void* s)
// {
//     kmt->create(pmm->alloc(sizeof(task_t)), "th5", th, "th5");
//     kmt->create(pmm->alloc(sizeof(task_t)), "th6", th, "th6");
//     kmt->create(pmm->alloc(sizeof(task_t)), "th7", th, "th7");
//     while (1) {
//         spin_lock(&lk);
//         assert(_intr_read() == 0);
//         //printf("This is %s running in CPU %d!\n", (const char*)s, _cpu());
//         spin_unlock(&lk);
//         for (volatile int i = 1; i < 100000; i++)
//             ;
//     }
// }

// static void tty_reader(void* arg)
// {
//     device_t* tty = dev->lookup(arg);
//     char cmd[128], resp[128], ps[16];
//     snprintf(ps, 16, "(%s) $ ", arg);
//     while (1) {
//         tty->ops->write(tty, 0, ps, strlen(ps));
//         int nread = tty->ops->read(tty, 0, cmd, sizeof(cmd) - 1);
//         cmd[nread] = '\0';
//         sprintf(resp, "tty reader task: got %d character(s).\n", strlen(cmd));
//         tty->ops->write(tty, 0, resp, strlen(resp));
//     }
// }

void vfs_test()
{
    int v1 = vfs->open("/f1", 0);
    assert(v1 == -1);
    int v2 = vfs->open("/f2", O_CREAT);
    int v3 = vfs->open("/f2", O_CREAT);
    assert(v2 == 0 && v3 == 1);
    vfs->close(v2);
    int v4 = vfs->open("/f3", O_CREAT);
    assert(v4 == 0);

    vfs->write(v3, "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789", 35);
    vfs->lseek(v3, 25, SEEK_SET);
    char s[128];
    memset(s, 0, 128);
    vfs->read(v3, s, 10);
    assert(strcmp("Z123456789", s) == 0);

    vfs->write(v4, "12345", 5);
    vfs->lseek(v4, 25, SEEK_END);
    vfs->write(v4, "54321", 5);
    vfs->lseek(v4, 29, SEEK_SET);
    vfs->read(v4, s, 6);
    assert(s[0] == 1 && strcmp(s + 1, "54321"));
    while (1)
        ;
}

static void os_init()
{
    pmm->init();
    kmt->init();

    kmt->spin_init(&trapLock, "trapLock");

    dev->init();
    // task_t* t1 = pmm->alloc(sizeof(task_t));
    // task_t* t2 = pmm->alloc(sizeof(task_t));
    // kmt->create(t1, "tty_reader", tty_reader, "tty1");
    // kmt->create(t2, "tty_reader", tty_reader, "tty2");

    // spin_init(&lk, NULL);

    //task_t* t1 = pmm->alloc(sizeof(task_t));
    // task_t* t2 = pmm->alloc(sizeof(task_t));
    //task_t* t3 = pmm->alloc(sizeof(task_t));

    //kmt->create(t1, "th1", th, "th1");
    // kmt->create(t2, "th2", th, "th2");
    //kmt->create(t3, "th3", th,NULL);
    // kmt->create(pmm->alloc(sizeof(task_t)), "th4", th4, "th4");

    // kmt->sem_init(&empty, "empty", 5);  // 缓冲区大小为 5
    // kmt->sem_init(&fill, "fill", 0);
    // for (int i = 0; i < 2; i++)  // 4 个生产者
    //     kmt->create(pmm->alloc(sizeof(task_t)), "producer", producer, NULL);
    // for (int i = 0; i <2; i++)  // 5 个消费者
    //     kmt->create(pmm->alloc(sizeof(task_t)), "consumer", consumer, NULL);

    vfs->init();

    kmt->create(pmm->alloc(sizeof(task_t)), "t1", vfs_test, NULL);
}

static void os_run()
{
    _intr_write(1);

    _yield();
}

_Context* os_trap(_Event ev, _Context* context)
{
    //kmt->spin_lock(&trapLock);
    _Context* next = NULL;
    for (int i = 0; i <= MAX_INTR; i++) {
        if (INTR[i].valid == 1 && (INTR[i].event == _EVENT_NULL || INTR[i].event == ev.event)) {
            _Context* r = INTR[i].handler(ev, context);
            panic_on(r && next, "returning multiple contexts");
            if (r) next = r;
        }
    }
    panic_on(!next, "returning NULL context");
    //kmt->spin_unlock(&trapLock);
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
