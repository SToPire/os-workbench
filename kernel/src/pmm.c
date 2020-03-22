#include <common.h>
#include <lock.h>

#define HDR_SIZE sizeof(spinlock_t) + sizeof(int) + sizeof(uintptr_t)
#define PAGE_SIZE (8 << 10)
typedef union page {
    struct {
        spinlock_t lock;  // 锁，用于串行化分配和并发的 free
        int obj_cnt;      // 页面中已分配的对象数，减少到 0 时回收页面
        union page* nxt;
        //list_head list;   // 属于同一个线程的页面的链表
    };  // 匿名结构体
    struct {
        uint8_t header[HDR_SIZE];
        uint8_t data[PAGE_SIZE - HDR_SIZE];
    } __attribute__((packed));
} page_t;

typedef struct __pmm_cache {
    page_t* list;
} cache_t;
static void* kalloc(size_t size)
{
    return NULL;
}

static void kfree(void* ptr)
{
}

static void pmm_init()
{
    uintptr_t pmsize = ((uintptr_t)_heap.end - (uintptr_t)_heap.start);
    printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, _heap.start, _heap.end);

    cache_t* kmem_cache = (cache_t*)_heap.end - 13;
    for (int i = 0; i < 13; ++i) kmem_cache[i].list = NULL;
    page_t* pages = (page_t*)_heap.start;
    const int PAGE_NUM = (((uintptr_t)kmem_cache & ((2 * PAGE_SIZE - 1) ^ (~PAGE_SIZE))) - (uintptr_t)_heap.start) / PAGE_SIZE;
    for (int i = 0; i < PAGE_NUM; ++i) {
        pages[i].nxt = &pages[i + 1];
        printf("%p\n", pages[i].nxt);
    }
    printf("%p\n", sizeof(page_t));
    //printf("%p %p %p %d\n", _heap.end, kmem_cache, ((uintptr_t)kmem_cache & ((2 * PAGE_SIZE - 1) ^ (~PAGE_SIZE))), PAGE_NUM);
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
