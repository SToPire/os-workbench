#include <common.h>
#include <lock.h>

#define HDR_SIZE 1024
#define PAGE_SIZE 8192
typedef union page {
    struct {
        spinlock_t lock;  // 锁，用于串行化分配和并发的 free
        int obj_cnt;      // 页面中已分配的对象数，减少到 0 时回收页面
        int maxUnit;      //  该页最大内存单元数
        bool full;
        int unitsize;  // 该页每个内存单元的大小

        uint64_t bitmap[112];
        int bitmapcnt;

        uintptr_t data_align;  // 对齐之后的数据域首地址
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

page_t* freePageHead;
cache_t* kmem_cache;
page_t* pages;

bool isUnitUsing(uint64_t* bitmap, bool num)
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
    if (kmem_cache[cachenum].list == NULL || kmem_cache[cachenum].list->full) {
        spin_lock(&freePageHead->lock);
        page_t* tmp = freePageHead;
        page_t* fPH_nxt = freePageHead->nxt;
        memset(tmp->header, 0, sizeof(tmp->header));
        tmp->nxt = kmem_cache[cachenum].list;
        tmp->unitsize = sz;
        if (sz == 2048 || sz == 4096)
            tmp->data_align = (uintptr_t)tmp->header + sz;
        else
            tmp->data_align = (uintptr_t)tmp->data;
        tmp->maxUnit = ((uintptr_t)tmp->header + PAGE_SIZE - (uintptr_t)tmp->data_align) / sz;

        kmem_cache[cachenum].list = tmp;
        freePageHead = fPH_nxt;
        spin_unlock(&freePageHead->lock);
    }
    page_t* curPage = kmem_cache[cachenum].list;
    spin_lock(&curPage->lock);
    if (sz == 4096) {
        curPage->full = true;
        curPage->obj_cnt = 1;
        //printf("%p\n", curPage->data_align);
        spin_unlock(&curPage->lock);
        return (void*)curPage->data_align;
    }
    //printf("%p %p %p %d\n", curPage->header, curPage->data, curPage->data_align, curPage->maxUnit);
    int oldcnt;
    if (curPage->bitmapcnt != 0)
        oldcnt = curPage->bitmapcnt - 1;
    else
        oldcnt = curPage->maxUnit - 1;
    while (oldcnt != curPage->bitmapcnt) {
        if (!isUnitUsing(curPage->bitmap, curPage->bitmapcnt)) {
            setUnit(curPage->bitmap, curPage->bitmapcnt, 1);
            void* ret = (void*)((uintptr_t)curPage->data_align + curPage->unitsize * curPage->bitmapcnt);
            curPage->bitmapcnt = (curPage->bitmapcnt + 1) % curPage->maxUnit;
            ++curPage->obj_cnt;
            if (curPage->obj_cnt == curPage->maxUnit) curPage->full = 1;
            spin_unlock(&curPage->lock);
            //printf("%p\n", ret);
            return ret;
        }
        curPage->bitmapcnt = (curPage->bitmapcnt + 1) % curPage->maxUnit;
    }
    spin_unlock(&curPage->lock);
    return NULL;
}

static void kfree(void* ptr)
{
    page_t* curPage = (page_t*)((uintptr_t)ptr & ((2 * PAGE_SIZE - 1) ^ (~PAGE_SIZE)));
    int num = ((uintptr_t)ptr - curPage->data_align) / curPage->unitsize;
    //printf("%p %p %d\n", ptr, curPage, num);
    setUnit(curPage->bitmap, num, 0);
    curPage->full = false;
    if(--curPage->obj_cnt ==0){
        // TODO
    }
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
    freePageHead = &pages[0];
    //printf("%p %p\n", pages[0].nxt, freePageHead);
    //printf("%d %d %d\n", HDR_SIZE, PAGE_SIZE, sizeof(page_t));
    //printf("%p %p %p %d\n", _heap.end, kmem_cache, ((uintptr_t)kmem_cache & ((2 * PAGE_SIZE - 1) ^ (~PAGE_SIZE))), PAGE_NUM);
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
