#include <user.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
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

superblock_t sb;
uint8_t* fs_head;

void addFAT(uint32_t from, uint32_t to)
{
    printf("%lx\n", sb.fat_head + sizeof(uint32_t) * from);
    memcpy(fs_head + sb.fat_head + sizeof(uint32_t) * from, (void*)(&to), sizeof(uint32_t));
}
uint32_t getNextFAT(uint32_t curBlk)
{
    uint32_t ret = 0;
    off_t offset = sb.fat_head + curBlk * sizeof(uint32_t);
    memcpy(fs_head + offset, (void*)(&ret), sizeof(uint32_t));
    return ret;
}
uint32_t getLastEntryBlk(uint32_t headBlk)
{
    uint32_t curBlk = headBlk;
    while (1) {
        uint32_t nxt = getNextFAT(curBlk);
        if (nxt == 0)
            return curBlk;
        else
            curBlk = nxt;
    }
}

void traverse(char* pathname)
{
    dinode_t dirInode;
    memset(&dirInode, 0, sizeof(dirInode));
    dirInode.stat.id = sb.fst_free_inode++;
    dirInode.stat.type = T_DIR;
    dirInode.stat.size = 2 * sizeof(struct ufs_dirent);
    dirInode.firstBlock = sb.fst_free_data_blk++;
    dirInode.refCnt = 1;

    uint32_t lstBlk = dirInode.firstBlock;

    DIR* dir = opendir(pathname);
    struct dirent* dir_entry;
    while ((dir_entry = readdir(dir)) != NULL) {
        uint32_t newInode = sb.fst_free_inode++;
        uint32_t newBlk = sb.fst_free_data_blk++;
        printf("%s %d %u %u\n", dir_entry->d_name, newInode, lstBlk, newBlk);
        addFAT(lstBlk, newBlk);
        struct ufs_dirent d;
        memset(&d, 0, sizeof(struct ufs_dirent));
        d.inode = newInode;
        strcpy(d.name, dir_entry->d_name);
        memcpy(fs_head + sb.data_head + sb.blk_size * lstBlk, (void*)(&d), sizeof(struct ufs_dirent));
        lstBlk = newBlk;

        // if (dir_entry->d_type == 8) {  // file
        //     char fullPath[512];
        //     sprintf(fullPath, "%s/%s", pathname, dir_entry->d_name);

        //     int fd = open(fullPath, O_RDWR);
        //     assert(fd > 0);
        //     struct stat statbuf;
        //     fstat(fd, &statbuf);
        //     printf("%s %d\n", dir_entry->d_name, (int)statbuf.st_size);

        //     dinode_t newDinode;
        //     memset(&newDinode, 0, sizeof(newDinode));
        //     newDinode.stat.id = ++sb.fst_free_inode;
        //     newDinode.stat.type = T_FILE;
        //     newDinode.stat.size = 0;
        // } else if (dir_entry->d_type == 4) {  // dir
        // }
    }
    memcpy(fs_head, (void*)(&sb), sizeof(sb));
}

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

    fs_head = disk + FS_OFFSET;

    sb.blk_size = 32U;
    sb.inode_size = 32U;
    sb.inode_head = sizeof(superblock_t);
    sb.fat_head = sb.inode_head + 1024U;
    sb.data_head = sb.fat_head + 1024U;
    sb.fst_free_data_blk = 0;
    sb.fst_free_inode = 0;

    // memcpy(fs_head, (void*)(&sb), sizeof(sb));

    // dinode_t rootInode;
    // memset(&rootInode, 0, sizeof(rootInode));
    // rootInode.stat.id = 0;
    // rootInode.stat.type = T_DIR;
    // rootInode.stat.size = 2*sizeof(struct ufs_dirent);
    // rootInode.firstBlock = 0;
    // rootInode.refCnt = 1;
    // memcpy(fs_head + sb.inode_head, (void*)(&rootInode), sizeof(rootInode));

    printf("disk:%p~%p\n", disk,disk+IMG_SIZE);
    traverse(argv[3]);

    munmap(disk, IMG_SIZE);
    close(fd);
}
