#include <common.h>
#include <devices.h>
#include <vfs.h>

inode_t* inodeSearch(inode_t* cur, const char* path)
{
    for (inode_t* ptr = cur->firstChild; ptr != NULL; ptr = ptr->nxtBrother) {
        if (strncmp(path, ptr->path, strlen(ptr->path) == 0)) {
            if (strlen(path) == strlen(ptr->path))
                return ptr;
            else
                return inodeSearch(ptr, path);
        }
    }
    return cur;
}

void inodeInsert(inode_t* parent, inode_t* cur)
{
    if (parent->firstChild == NULL)
        parent->firstChild = cur;
    else {
        inode_t* i;
        for (i = parent->firstChild; i->nxtBrother; i = i->nxtBrother)
            ;
        i->nxtBrother = cur;
    }
}

void inodeDelete(inode_t* parent, inode_t* child)
{
    if (child == parent->firstChild)
        parent->firstChild = child->nxtBrother;
    else {
        inode_t* i;
        for (i = parent->firstChild; i && i->nxtBrother != child; i = i->nxtBrother)
            ;
        if (i)
            i->nxtBrother = child->nxtBrother;
    }
}

#define FS_OFFSET 1 * 1024 * 1024
superblock_t sb;
device_t* sda;

void addFAT(uint32_t from, uint32_t to)
{
    sda->ops->write(sda, FS_OFFSET + sb.fat_head + sizeof(int32_t) * from, (void*)(&to), sizeof(uint32_t));
}
uint32_t getNextFAT(uint32_t curBlk)
{
    uint32_t ret = 0;
    off_t offset = sb.fat_head + curBlk * sizeof(uint32_t);
    sda->ops->read(sda, FS_OFFSET + offset, (void*)(&ret), sizeof(uint32_t));
    return ret;
}



inode_t* root;

void vfs_init()
{
    sda = dev->lookup("sda");
    sda->ops->read(sda, FS_OFFSET, &sb, sizeof(sb));
    printf("%u %u %u %u\n", sb.blk_size, sb.data_head, sb.fat_head, sb.fst_free_data_blk);

    root->firstChild = root->nxtBrother = NULL;
    root->parent = root;
    sprintf(root->path, "/");
    root->type = T_DIR;

    addFAT(0, 1);
    addFAT(1, 2);
    printf("ll::%u\n", getNextFAT(0));
    printf("ll::%u\n", getNextFAT(1));
}

MODULE_DEF(vfs) = {
    .init = vfs_init,
};
