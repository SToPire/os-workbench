#include <common.h>
#include <devices.h>
#include <vfs.h>

inode_t* inodeSearch(inode_t* cur, const char* path)
{
    for (inode_t* ptr = cur->firstChild; ptr != NULL; ptr = ptr->nxtBrother) {
        if(strncmp(path,ptr->path,strlen(ptr->path) == 0)){
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
        for (; i->nxtBrother; i = i->nxtBrother)
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
        for (; i && i->nxtBrother != child; i = i->nxtBrother)
            ;
        if (i)
            i->nxtBrother = child->nxtBrother;
    }
}
superblock_t sb;
device_t* sda;
void addFAT(uint32_t from, uint32_t to)
{
    
}


#define FS_OFFSET 1 * 1024 * 1024
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
}

MODULE_DEF(vfs) = {
    .init = vfs_init,
};
