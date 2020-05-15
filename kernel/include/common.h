#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>

#include<spinlock.h>
#include<kmt.h>

_Context* scheduler(_Event ev, _Context* _Context);