#include <common.h>
#include <lock.h>

#define HDR_SIZE 1024
#define PAGE_SIZE 8192
typedef union page {
    struct {
        spinlock_t lock;  // 锁，用于串行化分配和并发的 free
        int obj_cnt;      // 页面中已分配的对象数，减少到 0 时回收页面
        union page* nxt;
        uint64_t bitmap[112];
        int bitmapcnt;
        int unitsize;
        uintptr_t data_align;
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

page_t* freePageHead;
cache_t* kmem_cache;
page_t* pages;

bool isUnitValid(uint64_t* bitmap, bool num)
{
    return (bitmap[num / 64]) & (1 << (num % 64));
}
void setUnit(uint64_t* bitmap, int num, bool b)
{
    assert(b == 0 || b == 1);
    if (b == 0)
        bitmap[num / 64] |= (1 << (num % 64));
    else
        bitmap[num / 64] &= ~(1 << (num % 64));
}
static void* kalloc(size_t size)
{
    int sz = 1, cachenum = 0;
    while (sz < size) {
        sz <<= 1;
        ++cachenum;
    }
    if(kmem_cache[cachenum].list == NULL){ //TODO
        kmem_cache[cachenum].list = freePageHead;

        page_t* tmp = kmem_cache[cachenum].list;
        memset(tmp->header, 0, sizeof(tmp->header));
        tmp->unitsize = sz;
        tmp->data_align = ((uintptr_t)tmp->data + sz) & ~(2 * sz - 1);

        freePageHead = freePageHead->nxt;
    }
    page_t* curPage = kmem_cache[cachenum].list;
    printf("%p %p %p\n", curPage->header, curPage->data, curPage->data_align);

    return NULL;
}

static void kfree(void* ptr)
{
}

static void pmm_init()
{
    uintptr_t pmsize = ((uintptr_t)_heap.end - (uintptr_t)_heap.start);
    printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, _heap.start, _heap.end);

    kmem_cache = (cache_t*)_heap.end - 13;
    for (int i = 0; i < 13; ++i) kmem_cache[i].list = NULL;
    pages = (page_t*)_heap.start;
    const int PAGE_NUM = (((uintptr_t)kmem_cache & ((2 * PAGE_SIZE - 1) ^ (~PAGE_SIZE))) - (uintptr_t)_heap.start) / PAGE_SIZE;
    for (int i = 0; i < PAGE_NUM; ++i) {
        pages[i].nxt = &pages[i + 1];
        spin_init(&pages[i].lock);
        //printf("%p %p\n", pages[i].header, pages[i].data);
    }
    freePageHead = pages;
    //printf("%d %d %d\n", HDR_SIZE, PAGE_SIZE, sizeof(page_t));
    //printf("%p %p %p %d\n", _heap.end, kmem_cache, ((uintptr_t)kmem_cache & ((2 * PAGE_SIZE - 1) ^ (~PAGE_SIZE))), PAGE_NUM);
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
