#include <user.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <dirent.h>

#define FS_OFFSET 1 * 1024 * 1024

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

typedef struct _dinode {
    struct ufs_stat stat;
    uint32_t firstBlock;
    uint32_t refCnt;
} dinode_t;

int main(int argc, char* argv[])
{
    int fd;
    uint8_t* disk;

    int IMG_SIZE = atoi(argv[1]) * 1024 * 1024;

    // char* cwd = getcwd(NULL, 0);
    // char* newwd = malloc(strlen(cwd) - strlen("/tools") + 1);
    // strncpy(newwd, cwd, strlen(cwd) - strlen("/tools"));
    // assert(chdir(newwd) == 0);

    assert((fd = open(argv[2], O_RDWR)) > 0);
    assert((ftruncate(fd, IMG_SIZE)) == 0);
    assert((disk = mmap(NULL, IMG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) != MAP_FAILED);

    uint8_t* fs_head = disk + FS_OFFSET;

    superblock_t sb;
    sb.blk_size = 32U;
    sb.inode_size = 32U; 
    sb.inode_head = sizeof(superblock_t);
    sb.fat_head = sb.inode_head + 1024U;
    sb.data_head = sb.fat_head + 1024U;
    sb.fst_free_data_blk = 1;
    sb.fst_free_inode = 1;

    memcpy(fs_head, (void*)(&sb), sizeof(sb));

    dinode_t rootInode;
    memset(&rootInode, 0, sizeof(rootInode));
    rootInode.stat.id = 0;
    rootInode.stat.type = T_DIR;
    rootInode.stat.size = 0;
    rootInode.firstBlock = 0;
    rootInode.refCnt = 1;
    memcpy(fs_head + sb.inode_head, (void*)(&rootInode), sizeof(rootInode));

    DIR* mntPoint = opendir(argv[3]);

    munmap(disk, IMG_SIZE);
    close(fd);
}
