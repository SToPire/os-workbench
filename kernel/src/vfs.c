#include <common.h>
#include <devices.h>
#include <vfs.h>

#define FS_OFFSET 1 * 1024 * 1024

void vfs_init()
{
    superblock_t sb;
    device_t* sda = dev->lookup("sda");
}

MODULE_DEF(vfs) = {
    .init = vfs_init,
};
