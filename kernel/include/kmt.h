#include<am.h>
struct task{
    struct{
        char* name;
        struct task* next;
        _Context* context;
    };
    uint8_t stack[4096];
};
