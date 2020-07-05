#include <common.h>
#include <devices.h>
#include <vfs.h>

#define FS_OFFSET 1 * 1024 * 1024

void vfs_init()
{
    superblock_t sb;
    device_t* sda = dev->lookup("sda");
    sda->ops->write(sda, 0, &sb, sizeof(sb));

    printf("%u %u %u %u\n", sb.blk_size, sb.data_head, sb.fat_head, sb.fst_free_data_blk);
}

MODULE_DEF(vfs) = {
    .init = vfs_init,
};
