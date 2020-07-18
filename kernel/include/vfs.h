#ifndef __VFS_H__
#define __VFS_H__

#include<user.h>
typedef long off_t;

typedef struct _superblock {
    uint32_t blk_size;
    uint32_t inode_size;
    uint32_t inode_head;
    uint32_t fat_head;
    uint32_t data_head;
    uint32_t fst_free_data_blk;
    uint32_t fst_free_inode;
    uint8_t padding[4];
} superblock_t;

typedef union _entry{
    uint8_t Bytes[32]; // T_FILE
    struct ufs_dirent dir_entry; //T_DIR
} entry_t;

typedef struct _inode inode_t;
struct _inode {
    struct ufs_stat stat;
    char name[28];
    uint32_t firstBlock;

    inode_t* parent;
    inode_t* firstChild;
    inode_t* nxtBrother;
};

typedef struct _dinode{
    struct ufs_stat stat;
    char name[28];
    uint32_t firstBlock;
} dinode_t;

typedef struct _file {
    int fd;
    inode_t* inode;
    off_t offset;
    int valid;
} file_t;



#endif