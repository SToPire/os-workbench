#ifndef __VFS_H__
#define __VFS_H__

typedef long off_t;

typedef struct _superblock {
    uint32_t blk_size;
    uint32_t inode_head;
    uint32_t fat_head;
    uint32_t data_head;
    uint32_t fst_free_data_blk;
    uint8_t padding[12];
} superblock_t;

typedef union _entry{
    uint8_t Bytes[32]; // T_FILE
    struct {    //T_DIR
        uint32_t firstBlock;
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
    off_t offset;
    int valid;
} file_t;

int vfs_write(int fd, void* buf, int count);
int vfs_read(int fd, void* buf, int count);
int vfs_close(int fd);
int vfs_open(const char* pathname, int flags);

#endif