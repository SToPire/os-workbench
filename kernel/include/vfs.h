#ifndef __VFS_H__
#define __VFS_H__

typedef struct _superblock {
    uint32_t blk_size;
    uint32_t fat_head;
    uint32_t data_head;
    uint32_t fst_free_data_blk;
} superblock_t;

typedef union _entry{
    uint8_t Bytes[32]; // T_FILE
    struct {    //T_DIR
        uint32_t begin_blk;
        uint32_t type;
        uint8_t padding[24];
    } dir_entry;
} entry_t;

enum INODE_TYPE {
    T_INVALID,
    T_DIR,
    T_FILE,
};

typedef struct _inode inode_t;
struct _inode {
    uint32_t type;
    char path[128];
    uint32_t firstBlock;

    inode_t* parent;
    inode_t* firstChild;
    inode_t* nxtBrother;
};

typedef struct _file{
    int fd;
    //char path[128];
    inode_t* inode;
} file_t;

#endif