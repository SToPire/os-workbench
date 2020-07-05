#ifndef __VFS_H__
#define __VFS_H__

typedef struct superblock {
    uint32_t blk_size;
    uint32_t fat_head;
    uint32_t data_head;
    uint32_t fst_free_data_blk;
} superblock_t;

#endif