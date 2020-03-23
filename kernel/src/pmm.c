#include <common.h>
#include <lock.h>

#define HDR_SIZE 1024
#define PAGE_SIZE 8192
#define CPU_NUM 8
typedef union page {
    struct {
        spinlock_t lock;  // 锁，用于串行化分配和并发的 free
        int obj_cnt;      // 页面中已分配的对象数，减少到 0 时回收页面
        int maxUnit;      //  该页最大内存单元数
        bool full;
        int unitsize;  // 该页每个内存单元的大小
        int cachenum;

        uint64_t bitmap[112];
        int bitmapcnt;

        uintptr_t data_align;  // 对齐之后的数据域首地址

        int cpuid;
        union page* pre;
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
    spinlock_t cache_lock;
} cache_t;

page_t* freePageHead;
cache_t* kmem_cache[CPU_NUM];
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
spinlock_t L;
static void* kalloc(size_t size)
{
    int sz = 1, cachenum = 0;
    while (sz < size) {
        sz <<= 1;
        ++cachenum;
    }
    int cpu = _cpu();
    //printf("now_cpu:%d\n", cpu);
    bool new_page;

    page_t* curPage = kmem_cache[cpu][cachenum].list;

    if (curPage == NULL)
        new_page = true;
    else {
        printf("tes\n");
        while (curPage->full == true) {
            if(curPage->nxt) curPage = curPage->nxt;
            else
                new_page = true;
        }
    }
    if (new_page) {
        if (freePageHead == NULL) return NULL;
        spin_lock(&freePageHead->lock);
        page_t* tmp = freePageHead;
        freePageHead = freePageHead->nxt;
        spin_unlock(&tmp->lock);

        memset(tmp->header, 0, sizeof(tmp->header));
        spin_lock(&kmem_cache[cpu][cachenum].cache_lock);
        tmp->nxt = kmem_cache[cpu][cachenum].list;
        if (kmem_cache[cpu][cachenum].list) kmem_cache[cpu][cachenum].list->pre = tmp;
        spin_unlock(&kmem_cache[cpu][cachenum].cache_lock);

        tmp->unitsize = sz;
        tmp->cachenum = cachenum;
        tmp->cpuid = _cpu();

        if (sz == 2048 || sz == 4096)
            tmp->data_align = (uintptr_t)tmp->header + sz;
        else
            tmp->data_align = (uintptr_t)tmp->data;
        tmp->maxUnit = ((uintptr_t)tmp->header + PAGE_SIZE - (uintptr_t)tmp->data_align) / sz;

        spin_lock(&kmem_cache[cpu][cachenum].cache_lock);
        curPage = kmem_cache[cpu][cachenum].list = tmp;
        spin_unlock(&kmem_cache[cpu][cachenum].cache_lock);
    }

    spin_lock(&curPage->lock);
    //spin_lock(&L);

    int oldcnt = curPage->bitmapcnt;
    do {
        if (!isUnitUsing(curPage->bitmap, curPage->bitmapcnt)) {
            setUnit(curPage->bitmap, curPage->bitmapcnt, 1);
            void* ret = (void*)((uintptr_t)curPage->data_align + curPage->unitsize * curPage->bitmapcnt);
            curPage->bitmapcnt = (curPage->bitmapcnt + 1) % curPage->maxUnit;
            ++curPage->obj_cnt;
            if (curPage->obj_cnt == curPage->maxUnit) curPage->full = 1;
            //spin_unlock(&curPage->lock);
            printf("%d:%p\n", _cpu(), ret);
            return ret;
        }
        curPage->bitmapcnt = (curPage->bitmapcnt + 1) % curPage->maxUnit;
    } while (oldcnt != curPage->bitmapcnt);
    spin_unlock(&L);
    //spin_unlock(&curPage->lock);

    return NULL;
}

static void kfree(void* ptr)
{
    page_t* curPage = (page_t*)((uintptr_t)ptr & ((2 * PAGE_SIZE - 1) ^ (~PAGE_SIZE)));
    if (curPage->cpuid != _cpu()) return;
    spin_lock(&curPage->lock);
    int cpu = curPage->cpuid;
    int num = ((uintptr_t)ptr - curPage->data_align) / curPage->unitsize;
    setUnit(curPage->bitmap, num, 0);
    curPage->full = false;
    if (--curPage->obj_cnt == 0) {
        if (curPage->pre) {
            curPage->pre->nxt = curPage->nxt;
            if (curPage->nxt) curPage->nxt->pre = curPage->pre;
        } else {
            if (curPage->nxt) curPage->nxt->pre = NULL;
            spin_lock(&kmem_cache[cpu][curPage->cachenum].cache_lock);
            kmem_cache[cpu][curPage->cachenum].list = curPage->nxt;
            spin_unlock(&kmem_cache[cpu][curPage->cachenum].cache_lock);
        }
        curPage->nxt = freePageHead;
        freePageHead = curPage;
    }
    spin_unlock(&curPage->lock);
}

static void pmm_init()
{
    uintptr_t pmsize = ((uintptr_t)_heap.end - (uintptr_t)_heap.start);
    printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, _heap.start, _heap.end);

    for (int i = CPU_NUM - 1; i >= 0; --i)
        kmem_cache[i] = (cache_t*)_heap.end - (CPU_NUM - i) * 13;
    for (int i = 0; i < CPU_NUM; ++i) {
        for (int j = 0; j < 13; ++j) {
            kmem_cache[i][j].list = NULL;
            spin_init(&kmem_cache[i][j].cache_lock);
        }
    }

    pages = (page_t*)_heap.start;
    const int PAGE_NUM = (((uintptr_t)(_heap.end - CPU_NUM * 13) & ((2 * PAGE_SIZE - 1) ^ (~PAGE_SIZE))) - (uintptr_t)_heap.start) / PAGE_SIZE;
    for (int i = 0; i < PAGE_NUM; ++i) {
        pages[i].nxt = &pages[i + 1];
        spin_init(&pages[i].lock);
    }
    freePageHead = &pages[0];
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = kalloc,
    .free = kfree,
};
