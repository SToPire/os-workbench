#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>

#include<spinlock.h>
#include<kmt.h>
#include<sem.h>

struct cpu_local {
    task_t* current;
} cpu_local[8];
#define current cpu_local[_cpu()].current

_Context* scheduler(_Event ev, _Context* _Context);