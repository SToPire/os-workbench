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
    // uint32_t iNum;
    // uint32_t type;
    struct ufs_stat stat;
    char name[28];
    uint32_t firstBlock;

    inode_t* parent;
    inode_t* firstChild;
    inode_t* nxtBrother;
};

typedef struct _dinode{
    // uint32_t iNum;
    // uint32_t type;
    struct ufs_stat stat;
    char name[28];
    uint32_t firstBlock;
} dinode_t;

typedef struct _file {
    int fd;
    //char path[128];
    inode_t* inode;
    off_t offset;
    int valid;
} file_t;

int vfs_write(int fd, void* buf, int count);
int vfs_read(int fd, void* buf, int count);
int vfs_close(int fd);
int vfs_open(const char* pathname, int flags);

#endif