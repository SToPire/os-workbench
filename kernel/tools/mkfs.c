#include <user.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#define FS_OFFSET 1 * 1024 * 1024

typedef struct superblock {
    uint32_t blk_size;
    uint32_t fat_head;
    uint32_t data_head;
    uint32_t fst_free_data_blk;
} superblock_t;

int main(int argc, char* argv[])
{
    int fd;
    uint8_t* disk;

    int IMG_SIZE = atoi(argv[1]) * 1024 * 1024;

    char* cwd = getcwd(NULL, 0);
    char* newwd = malloc(strlen(cwd) - strlen("/tools") + 1);
    strncpy(newwd, cwd, strlen(cwd) - strlen("/tools"));
    assert(chdir(newwd) == 0);

    assert((fd = open(argv[2], O_RDWR)) > 0);
    assert((ftruncate(fd, IMG_SIZE)) == 0);
    assert((disk = mmap(NULL, IMG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) != MAP_FAILED);

    uint8_t* fs_head = disk + FS_OFFSET;

    superblock_t sb;
    sb.blk_size = 32;
    sb.fat_head = 16;
    sb.data_head = 1024;
    sb.fst_free_data_blk = 1;

    memcpy(fs_head, (void*)(&sb), sizeof(sb));
    munmap(disk, IMG_SIZE);
    close(fd);
}
