#include <user.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#include "../include/devices.h"

#define FS_OFFSET 1 * 1024 * 1024

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

    //uint8_t* fs_head = disk + FS_OFFSET;
    //fs_head[0] = 'A';

    munmap(disk, IMG_SIZE);
    close(fd);
}
