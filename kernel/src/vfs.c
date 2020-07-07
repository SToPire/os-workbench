#include <common.h>
#include <devices.h>
#include <vfs.h>
#include <user.h>

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
    root->firstBlock = sb.fst_free_data_blk;
    ++sb.fst_free_data_blk;
    sda->ops->write(sda, FS_OFFSET, (void*)(&sb), sizeof(sb));

    // entry_t e;
    // memset(&e, 0, sizeof(e));
    // e.Bytes[0] = 0xff;
    // sda->ops->write(sda, FS_OFFSET + sb.data_head, &e, sizeof(e));
    // printf("\n%x", FS_OFFSET + sb.data_head);

    vfs_open("/a", O_CREAT);
}

// int vfs_write(int fd, void* buf, int count)
// {
// }

// int vfs_read(int fd, void* buf, int count)
// {
// }

// int vfs_close(int fd)
// {

// }

int vfs_open(const char* pathname, int flags)
{
    if (flags & O_CREAT) {
        if (pathname[0] == '/') {
            int i = strlen(pathname);
            while (pathname[i] != '/') --i;
            char filename[128], dirname[128];
            strcpy(filename, pathname + i + 1);
            strncpy(dirname, pathname, i + 1);
            dirname[i + 1] = '\0';
            printf("%s %s\n", dirname, filename);

            inode_t* ip = inodeSearch(root, dirname);
            if (strcmp(ip->path,dirname) != 0) return -1;
            printf("%s\n", ip->path);

            uint32_t entryBlkNO = getLastEntryBlk(ip->firstBlock);
            printf("entryBlk:%u\n", entryBlkNO);
            addFAT(entryBlkNO, sb.fst_free_data_blk);
            ++sb.fst_free_data_blk;
            sda->ops->write(sda, FS_OFFSET, (void*)(&sb), sizeof(sb));

            entry_t newEntry;
            memset(&newEntry, 0, sizeof(newEntry));
            newEntry.dir_entry.type = T_DIR;
            newEntry.dir_entry.begin_blk = sb.fst_free_data_blk;
            ++sb.fst_free_data_blk;
            sda->ops->write(sda, FS_OFFSET, (void*)(&sb), sizeof(sb));

            sda->ops->write(sda, FS_OFFSET + sb.data_head + entryBlkNO * sb.blk_size, (void*)(&newEntry), sizeof(newEntry));
        }
    }
    return 0;
}

MODULE_DEF(vfs) = {
    .init = vfs_init,
};
