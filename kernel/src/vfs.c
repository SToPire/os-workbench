#include <common.h>
#include <devices.h>
#include <vfs.h>

#define current cpu_local[_cpu()].current
#define getFileFromFD(fd) current->fds[fd];

/* ---------- Inode Operation ----------*/
inode_t* inodeSearch(inode_t* cur, const char* path)
{
    if (strcmp(path, "/") == 0) {
        if (strcmp(cur->name, "/") == 0)
            return cur;
        else
            return (void*)(-1);
    }

    if (path[strlen(path) - 1] == '/') {
        char* myPath = pmm->alloc(strlen(path));
        strncpy(myPath, path, strlen(path) - 1);
        return inodeSearch(cur, myPath);
    }

    assert(path[0] == '/');
    char* curName = pmm->alloc(strlen(path) + 1);
    int i = 1;
    while (i < strlen(path) && path[i] != '/') ++i;
    if (i == strlen(path)) {
        strcpy(curName, path + 1);
    } else {
        strncpy(curName, path + 1, i - 1);
    }
    for (inode_t* ptr = cur->firstChild; ptr != NULL; ptr = ptr->nxtBrother) {
        if (strcmp(ptr->name, curName) == 0) {
            if (i == strlen(path))
                return ptr;
            else
                return inodeSearch(ptr, path + i);
        }
    }
    return (void*)(-1);
}

void inodeInsert(inode_t* parent, inode_t* cur)
{
    cur->parent = parent;
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
/* ---------- Inode Operation ----------*/

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

void writeEntry(uint32_t NO, entry_t* e)
{
    sda->ops->write(sda, FS_OFFSET + sb.data_head + NO * sb.blk_size, (void*)(e), sizeof(entry_t));
}
void readEntry(uint32_t NO, entry_t* e)
{
    sda->ops->read(sda, FS_OFFSET + sb.data_head + NO * sb.blk_size, (void*)(e), sizeof(entry_t));
}

inode_t* root;

void vfs_init()
{
    sda = dev->lookup("sda");
    sda->ops->read(sda, FS_OFFSET, &sb, sizeof(sb));
    printf("blk_size:%u inode_size:%u inode_head:%u fat_head:%u data_head:%u fst_free_data_blk:%u fst_free_inode:%u\n", sb.blk_size, sb.inode_size, sb.inode_head, sb.fat_head, sb.data_head, sb.fst_free_data_blk, sb.fst_free_inode);

    dinode_t* d_root = pmm->alloc(sizeof(dinode_t));
    sda->ops->read(sda, FS_OFFSET + sb.inode_head, (void*)d_root, sizeof(dinode_t));
    root = pmm->alloc(sizeof(inode_t));
    root->parent = root;
    root->firstChild = root->nxtBrother = NULL;
    root->firstBlock = d_root->firstBlock;
    memcpy(&(root->stat), &(d_root->stat), sizeof(root->stat));
    strcpy(root->name, d_root->name);
}

int vfs_write(int fd, void* buf, int count)
{
    file_t* file = getFileFromFD(fd);
    off_t offset = file->offset;
    uint32_t curBlk = file->inode->firstBlock;
    while (offset >= sb.blk_size) {  //move to required block
        uint32_t nxtBlk = getNextFAT(curBlk);
        offset -= sb.blk_size;
        if (nxtBlk == 0) {
            addFAT(curBlk, sb.fst_free_data_blk);
            ++sb.fst_free_data_blk;
            sda->ops->write(sda, FS_OFFSET, (void*)(&sb), sizeof(sb));
            curBlk = getNextFAT(curBlk);
        } else
            curBlk = nxtBlk;
    }

    int writeCnt = 0;
    while (count > 0) {
        entry_t entry;
        readEntry(curBlk, &entry);
        if (offset + count <= sb.blk_size) {
            memcpy(entry.Bytes + offset, buf + writeCnt, count);
            offset = (offset + count) % sb.blk_size;
            writeCnt += count;
            count = 0;
            writeEntry(curBlk, &entry);
        } else {
            memcpy(entry.Bytes + offset, buf + writeCnt, sb.blk_size - offset);
            writeCnt += (sb.blk_size - offset);
            count -= (sb.blk_size - offset);
            offset = 0;
            writeEntry(curBlk, &entry);

            uint32_t nxtBlk = getNextFAT(curBlk);
            if (nxtBlk == 0) {  // must add a new block
                addFAT(curBlk, sb.fst_free_data_blk);
                ++sb.fst_free_data_blk;
                sda->ops->write(sda, FS_OFFSET, (void*)(&sb), sizeof(sb));
                curBlk = getNextFAT(curBlk);
            } else {
                curBlk = nxtBlk;
            }
        }
    }

    if (file->inode->stat.size < file->offset + writeCnt)
        file->inode->stat.size = file->offset + writeCnt;
    file->offset += writeCnt;
    dinode_t newDinode;
    memcpy(&newDinode, file->inode, sizeof(newDinode));
    sda->ops->write(sda, FS_OFFSET + sb.inode_head + file->inode->stat.id * sb.inode_size, &newDinode, sizeof(dinode_t));

    return writeCnt;
}

int vfs_read(int fd, void* buf, int count)
{
    file_t* file = getFileFromFD(fd);
    if (file->inode->stat.size <= file->offset) return 0;
    if (file->inode->stat.size - file->offset < count) count = file->inode->stat.size - file->offset;
    off_t offset = file->offset;
    uint32_t curBlk = file->inode->firstBlock;
    while (offset >= sb.blk_size) {
        curBlk = getNextFAT(curBlk);
        offset -= sb.blk_size;
        if (curBlk == 0) return 0;
    }

    int readCnt = 0;
    while (count > 0) {
        entry_t entry;
        readEntry(curBlk, &entry);
        if (offset + count <= sb.blk_size) {
            memcpy(buf + readCnt, entry.Bytes + offset, count);
            offset = (offset + count) % sb.blk_size;
            readCnt += count;
            count = 0;
        } else {
            memcpy(buf + readCnt, entry.Bytes + offset, sb.blk_size - offset);
            readCnt += (sb.blk_size - offset);
            count -= (sb.blk_size - offset);
            offset = 0;

            curBlk = getNextFAT(curBlk);
            assert(curBlk != 0);
        }
    }
    file->offset += readCnt;
    return readCnt;
}

int vfs_close(int fd)
{
    current->fds[fd]->valid = 0;
    return 0;
}

int vfs_open(const char* pathname, int flags)
{
    if (flags & O_CREAT) {
        if (pathname[0] == '/') {
            int i = strlen(pathname);
            while (pathname[i] != '/') --i;
            char filename[128], dirname[128];
            memset(filename, 0, 128);
            memset(dirname, 0, 128);
            strcpy(filename, pathname + i + 1);
            strncpy(dirname, pathname, i + 1);
            dirname[i + 1] = '\0';
            //printf("dirname:%s filename:%s\n", dirname, filename);

            inode_t* ip = inodeSearch(root, dirname);
            if (ip == (void*)(-1)) return -1;
            ip->stat.size += sizeof(struct ufs_dirent);
            dinode_t newParentDinode;
            memcpy(&newParentDinode, ip, sizeof(dinode_t));
            sda->ops->write(sda, FS_OFFSET + sb.inode_head + ip->stat.id * sb.inode_size, &newParentDinode, sizeof(dinode_t));

            uint32_t entryBlkNO = getLastEntryBlk(ip->firstBlock);
            addFAT(entryBlkNO, sb.fst_free_data_blk);
            ++sb.fst_free_data_blk;
            sda->ops->write(sda, FS_OFFSET, (void*)(&sb), sizeof(sb));

            inode_t* newInode = pmm->alloc(sizeof(inode_t));
            memset(newInode, 0, sizeof(inode_t));
            newInode->stat.id = sb.fst_free_inode;
            newInode->stat.type = T_FILE;
            newInode->stat.size = 0;
            newInode->firstBlock = sb.fst_free_data_blk;
            strcpy(newInode->name, filename);

            inodeInsert(ip, newInode);

            dinode_t* newDinode = pmm->alloc(sizeof(dinode_t));
            memset(newDinode, 0, sizeof(dinode_t));
            newDinode->stat.id = sb.fst_free_inode;
            newDinode->stat.type = T_FILE;
            newDinode->stat.size = 0;
            newDinode->firstBlock = sb.fst_free_data_blk;
            strcpy(newDinode->name, filename);
            sda->ops->write(sda, FS_OFFSET + sb.inode_head + sb.fst_free_inode * sb.inode_size, (void*)newDinode, sizeof(dinode_t));
            ++sb.fst_free_inode;
            ++sb.fst_free_data_blk;
            sda->ops->write(sda, FS_OFFSET, (void*)(&sb), sizeof(sb));

            entry_t newEntry;
            memset(&newEntry, 0, sizeof(newEntry));
            newEntry.dir_entry.inode = newInode->stat.id;
            strcpy(newEntry.dir_entry.name, newInode->name);
            writeEntry(entryBlkNO, &newEntry);

            file_t* newFile = pmm->alloc(sizeof(file_t));
            int free_fd = 0;
            for (; free_fd < 128; free_fd++) {
                if (current->fds[free_fd] == NULL || current->fds[free_fd]->valid == 0) break;
            }
            newFile->fd = free_fd;
            newFile->inode = newInode;
            newFile->offset = 0;
            newFile->valid = 1;
            current->fds[newFile->fd] = newFile;
            return newFile->fd;
        }
    } else {   //do not create file
        printf("else\n");
        inode_t* newInode = inodeSearch(root, pathname);
        printf("id:%d\n", newInode->stat.id);
    }
    return -1;
}

int vfs_lseek(int fd, int offset, int whence)
{
    file_t* file = getFileFromFD(fd);
    if (whence == SEEK_CUR) {
        file->offset = file->offset + offset;
    } else if (whence == SEEK_END) {
        file->offset = file->inode->stat.size + offset;
    } else if (whence == SEEK_SET) {
        file->offset = offset;
    } else {
        assert(0);
    }

    assert(file->offset >= 0);
    return file->offset;
}

MODULE_DEF(vfs) = {
    .init = vfs_init,
    .open = vfs_open,
    .close = vfs_close,
    .write = vfs_write,
    .read = vfs_read,
    .lseek = vfs_lseek,
};
