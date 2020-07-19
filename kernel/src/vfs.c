#include <common.h>
#include <devices.h>
#include <vfs.h>

#define current cpu_local[_cpu()].current
#define getFileFromFD(fd) ofiles[current->fds[fd]];
#define NUM_OFILE 128
file_t* ofiles[NUM_OFILE];
int cnt_ofile;

/* ---------- Inode Operation ---------- */
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
    printf("ss:%s\n", curName);
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
/* ---------- Inode Operation ---------- */
/* ---------- UFS ---------- */
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

void getStatFromDinode(uint32_t dInodeNum, struct ufs_stat* status)
{
    dinode_t d;
    sda->ops->read(sda, FS_OFFSET + sb.inode_head + sb.inode_size * dInodeNum, &d, sizeof(dinode_t));
    status->id = d.stat.id;
    status->size = d.stat.size;
    status->type = d.stat.type;
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

void ufs_init()
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
    root->dInodeNum = 0;
    //memcpy(&(root->stat), &(d_root->stat), sizeof(root->stat));
    strcpy(root->name, "/");

    //. && ..
    int tmp = sb.fst_free_data_blk;
    addFAT(root->firstBlock, sb.fst_free_data_blk);
    ++sb.fst_free_data_blk;
    addFAT(tmp, sb.fst_free_data_blk);
    ++sb.fst_free_data_blk;
    sda->ops->write(sda, FS_OFFSET, (void*)(&sb), sizeof(sb));
    entry_t e1, e2;
    memset(&e1, 0, sizeof(e1));
    memset(&e2, 0, sizeof(e2));
    e1.dir_entry.inode = root->dInodeNum;
    e2.dir_entry.inode = root->dInodeNum;
    strcpy(e1.dir_entry.name, ".");
    strcpy(e2.dir_entry.name, "..");
    writeEntry(root->firstBlock, &e1);
    writeEntry(tmp, &e2);
}

int ufs_write(int fd, void* buf, int count)
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

    dinode_t newDinode;
    sda->ops->read(sda, FS_OFFSET + sb.inode_head + sb.inode_size * file->inode->dInodeNum, &newDinode, sizeof(dinode_t));
    if (newDinode.stat.size < file->offset + writeCnt)
        newDinode.stat.size = file->offset + writeCnt;
    file->offset += writeCnt;
    sda->ops->write(sda, FS_OFFSET + sb.inode_head + sb.inode_size * file->inode->dInodeNum, &newDinode, sizeof(dinode_t));

    return writeCnt;
}

int ufs_read(int fd, void* buf, int count)
{
    file_t* file = getFileFromFD(fd);
    struct ufs_stat* status = pmm->alloc(sizeof(struct ufs_stat));
    getStatFromDinode(file->inode->dInodeNum, status);
    if (status->size <= file->offset) return 0;
    if (status->size - file->offset < count) count = status->size - file->offset;
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

int ufs_close(int fd)
{
    ofiles[current->fds[fd]]->valid = 0;
    current->fds[fd] = -1;
    --cnt_ofile;
    return 0;
}

int ufs_open(const char* pathname, int flags)
{
    char absolutePathname[128];
    if (pathname[0] == '/')
        strcpy(absolutePathname, pathname);
    else
        sprintf(absolutePathname, "%s%s", current->cwd, pathname);

    if ((flags & O_CREAT) && inodeSearch(root, absolutePathname) == (void*)-1) {
        int i = strlen(absolutePathname);
        while (absolutePathname[i] != '/') --i;
        char filename[128], dirname[128];
        memset(filename, 0, 128);
        memset(dirname, 0, 128);
        strcpy(filename, absolutePathname + i + 1);
        strncpy(dirname, absolutePathname, i + 1);
        dirname[i + 1] = '\0';
        //printf("dirname:%s filename:%s\n", dirname, filename);

        inode_t* ip = inodeSearch(root, dirname);
        if (ip == (void*)(-1)) return -1;
        struct ufs_stat* status = pmm->alloc(sizeof(struct ufs_stat));
        getStatFromDinode(ip->dInodeNum, status);
        status->size += sizeof(struct ufs_dirent);
        dinode_t newParentDinode;
        newParentDinode.firstBlock = ip->firstBlock;
        newParentDinode.refCnt = 1;
        memcpy(&(newParentDinode.stat), status, sizeof(struct ufs_stat));
        sda->ops->write(sda, FS_OFFSET + sb.inode_head + status->id * sb.inode_size, &newParentDinode, sizeof(dinode_t));

        uint32_t entryBlkNO = getLastEntryBlk(ip->firstBlock);
        addFAT(entryBlkNO, sb.fst_free_data_blk);
        ++sb.fst_free_data_blk;
        sda->ops->write(sda, FS_OFFSET, (void*)(&sb), sizeof(sb));

        inode_t* newInode = pmm->alloc(sizeof(inode_t));
        memset(newInode, 0, sizeof(inode_t));
        newInode->dInodeNum = sb.fst_free_inode;
        newInode->firstBlock = sb.fst_free_data_blk;
        strcpy(newInode->name, filename);

        inodeInsert(ip, newInode);

        dinode_t* newDinode = pmm->alloc(sizeof(dinode_t));
        memset(newDinode, 0, sizeof(dinode_t));
        newDinode->stat.id = sb.fst_free_inode;
        newDinode->stat.type = T_FILE;
        newDinode->stat.size = 0;
        newDinode->firstBlock = sb.fst_free_data_blk;
        newDinode->refCnt = 1;
        sda->ops->write(sda, FS_OFFSET + sb.inode_head + sb.fst_free_inode * sb.inode_size, (void*)newDinode, sizeof(dinode_t));
        ++sb.fst_free_inode;
        ++sb.fst_free_data_blk;
        sda->ops->write(sda, FS_OFFSET, (void*)(&sb), sizeof(sb));

        entry_t newEntry;
        memset(&newEntry, 0, sizeof(newEntry));
        newEntry.dir_entry.inode = newDinode->stat.id;
        strcpy(newEntry.dir_entry.name, newInode->name);
        writeEntry(entryBlkNO, &newEntry);

        file_t* newFile = pmm->alloc(sizeof(file_t));
        int free_fd = 0;
        for (; free_fd < 128; free_fd++) {
            if (current->fds[free_fd] == -1) break;
        }
        if (free_fd == 128) return -1;
        newFile->fd = free_fd;
        newFile->inode = newInode;
        newFile->offset = 0;
        newFile->valid = 1;
        int fst_ofile_ptr = 0;
        for (; fst_ofile_ptr < NUM_OFILE; fst_ofile_ptr++) {
            if (ofiles[fst_ofile_ptr] == NULL || ofiles[fst_ofile_ptr]->valid == 0) break;
        }
        current->fds[free_fd] = fst_ofile_ptr;
        ofiles[fst_ofile_ptr] = newFile;
        ++cnt_ofile;
        return newFile->fd;
    } else {  //do not create file
        inode_t* existInode = inodeSearch(root, absolutePathname);
        if (existInode == (void*)(-1)) return -1;

        file_t* newFile = pmm->alloc(sizeof(file_t));
        int free_fd = 0;
        for (; free_fd < 128; free_fd++) {
            if (current->fds[free_fd] == -1) break;
        }
        if (free_fd == 128) return -1;
        newFile->fd = free_fd;
        newFile->inode = existInode;
        newFile->offset = 0;
        newFile->valid = 1;
        int fst_ofile_ptr = 0;
        for (; fst_ofile_ptr < NUM_OFILE; fst_ofile_ptr++) {
            if (ofiles[fst_ofile_ptr] == NULL || ofiles[fst_ofile_ptr]->valid == 0) break;
        }
        current->fds[free_fd] = fst_ofile_ptr;
        ofiles[fst_ofile_ptr] = newFile;
        ++cnt_ofile;
        return newFile->fd;
    }
    return -1;
}

int ufs_lseek(int fd, int offset, int whence)
{
    file_t* file = getFileFromFD(fd);
    if (whence == SEEK_CUR) {
        file->offset = file->offset + offset;
    } else if (whence == SEEK_END) {
        struct ufs_stat* status = pmm->alloc(sizeof(struct ufs_stat));
        getStatFromDinode(file->inode->dInodeNum, status);
        file->offset = status->size + offset;
    } else if (whence == SEEK_SET) {
        file->offset = offset;
    } else {
        assert(0);
    }

    assert(file->offset >= 0);
    return file->offset;
}

int ufs_link(const char* oldpath, const char* newpath)
{
    char absoluteOldpath[128], absoluteNewpath[128];
    if (oldpath[0] == '/')
        strcpy(absoluteOldpath, oldpath);
    else
        sprintf(absoluteOldpath, "%s%s", current->cwd, oldpath);
    if (newpath[0] == '/')
        strcpy(absoluteNewpath, newpath);
    else
        sprintf(absoluteNewpath, "%s%s", current->cwd, newpath);
    int i = strlen(absoluteNewpath);
    while (absoluteNewpath[i] != '/') --i;
    char filename[128], dirname[128];
    memset(filename, 0, 128);
    memset(dirname, 0, 128);
    strcpy(filename, absoluteNewpath + i + 1);
    strncpy(dirname, absoluteNewpath, i + 1);
    dirname[i + 1] = '\0';

    if (inodeSearch(root, absoluteNewpath) != (void*)(-1)) return -1;

    inode_t* ip = inodeSearch(root, dirname);
    if (ip == (void*)(-1)) return -1;

    inode_t* oldInode = inodeSearch(root, absoluteOldpath);
    if (oldInode == (void*)(-1)) return -1;

    inode_t* newInode = pmm->alloc(sizeof(inode_t));
    newInode->dInodeNum = oldInode->dInodeNum;
    newInode->firstBlock = oldInode->firstBlock;
    newInode->firstChild = newInode->nxtBrother = newInode->parent = NULL;
    strcpy(newInode->name, filename);
    inodeInsert(ip, newInode);

    dinode_t newDinode;
    sda->ops->read(sda, FS_OFFSET + sb.inode_head + sb.inode_size * newInode->dInodeNum, &newDinode, sizeof(dinode_t));
    ++newDinode.refCnt;
    sda->ops->write(sda, FS_OFFSET + sb.inode_head + sb.inode_size * newInode->dInodeNum, &newDinode, sizeof(dinode_t));

    return 0;
}

int ufs_unlink(const char* pathname)
{
    char absolutePathname[128];
    if (pathname[0] == '/')
        strcpy(absolutePathname, pathname);
    else
        sprintf(absolutePathname, "%s%s", current->cwd, pathname);

    int i = strlen(absolutePathname);
    while (absolutePathname[i] != '/') --i;
    char dirname[128];
    memset(dirname, 0, 128);
    strncpy(dirname, absolutePathname, i + 1);
    dirname[i + 1] = '\0';

    inode_t* pinode = inodeSearch(root, dirname);
    inode_t* inode = inodeSearch(root, absolutePathname);
    if (inode == (void*)(-1) || pinode == (void*)(-1)) return -1;
    /* TBD:  unlink an opening file*/
    /* TBD:  reusing block & dinode*/

    dinode_t newDinode;
    sda->ops->read(sda, FS_OFFSET + sb.inode_head + sb.inode_size * inode->dInodeNum, &newDinode, sizeof(dinode_t));
    --newDinode.refCnt;
    sda->ops->write(sda, FS_OFFSET + sb.inode_head + sb.inode_size * inode->dInodeNum, &newDinode, sizeof(dinode_t));

    inodeDelete(pinode, inode);
    return 0;
}

int ufs_fstat(int fd, struct ufs_stat* buf)
{
    file_t* file = getFileFromFD(fd);
    getStatFromDinode(file->inode->dInodeNum, buf);
    return 0;
}

int ufs_mkdir(const char* pathname)
{
    char absolutePathname[128];
    if (pathname[0] == '/')
        strcpy(absolutePathname, pathname);
    else
        sprintf(absolutePathname, "%s%s", current->cwd, pathname);
    if (inodeSearch(root, absolutePathname) != (void*)(-1)) return -1;

    int i = strlen(absolutePathname);
    while (absolutePathname[i] != '/') --i;
    char dirname[128], pdirname[128];
    memset(pdirname, 0, 128);
    memset(dirname, 0, 128);
    strcpy(dirname, absolutePathname + i + 1);
    strncpy(pdirname, absolutePathname, i + 1);
    pdirname[i + 1] = '\0';

    inode_t* pInode = inodeSearch(root, pdirname);
    if (pInode == (void*)(-1)) return -1;
    struct ufs_stat* status = pmm->alloc(sizeof(struct ufs_stat));
    getStatFromDinode(pInode->dInodeNum, status);
    status->size += sizeof(struct ufs_dirent);
    dinode_t newParentDinode;
    newParentDinode.firstBlock = pInode->firstBlock;
    newParentDinode.refCnt = 1;
    memcpy(&(newParentDinode.stat), status, sizeof(struct ufs_stat));
    sda->ops->write(sda, FS_OFFSET + sb.inode_head + status->id * sb.inode_size, &newParentDinode, sizeof(dinode_t));

    uint32_t entryBlkNO = getLastEntryBlk(pInode->firstBlock);
    addFAT(entryBlkNO, sb.fst_free_data_blk);
    ++sb.fst_free_data_blk;
    sda->ops->write(sda, FS_OFFSET, (void*)(&sb), sizeof(sb));

    inode_t* newInode = pmm->alloc(sizeof(inode_t));
    newInode->dInodeNum = sb.fst_free_inode;
    newInode->firstBlock = sb.fst_free_data_blk;
    strcpy(newInode->name, dirname);

    inodeInsert(pInode, newInode);

    dinode_t* newDinode = pmm->alloc(sizeof(dinode_t));
    newDinode->stat.id = sb.fst_free_inode;
    newDinode->stat.type = T_DIR;
    newDinode->stat.size = 2 * sizeof(struct ufs_dirent);
    newDinode->firstBlock = sb.fst_free_data_blk;
    newDinode->refCnt = 1;
    sda->ops->write(sda, FS_OFFSET + sb.inode_head + sb.fst_free_inode * sb.inode_size, (void*)newDinode, sizeof(dinode_t));
    ++sb.fst_free_inode;
    ++sb.fst_free_data_blk;
    sda->ops->write(sda, FS_OFFSET, (void*)(&sb), sizeof(sb));

    entry_t newEntry;
    memset(&newEntry, 0, sizeof(newEntry));
    newEntry.dir_entry.inode = newDinode->stat.id;
    strcpy(newEntry.dir_entry.name, newInode->name);
    writeEntry(entryBlkNO, &newEntry);

    //. && ..
    int tmp = sb.fst_free_data_blk;
    addFAT(newDinode->firstBlock, sb.fst_free_data_blk);
    ++sb.fst_free_data_blk;
    addFAT(tmp, sb.fst_free_data_blk);
    ++sb.fst_free_data_blk;
    sda->ops->write(sda, FS_OFFSET, (void*)(&sb), sizeof(sb));
    entry_t e1, e2;
    memset(&e1, 0, sizeof(e1));
    memset(&e2, 0, sizeof(e2));
    e1.dir_entry.inode = newInode->dInodeNum;
    e2.dir_entry.inode = pInode->dInodeNum;
    strcpy(e1.dir_entry.name, ".");
    strcpy(e2.dir_entry.name, "..");
    writeEntry(newDinode->firstBlock, &e1);
    writeEntry(tmp, &e2);

    return 0;
}

int ufs_chdir(const char* path)
{
    char absolutePathname[128];
    if (path[0] == '/')
        strcpy(absolutePathname, path);
    else
        sprintf(absolutePathname, "%s%s", current->cwd, path);

    if (absolutePathname[strlen(absolutePathname) - 1] != '/') {
        absolutePathname[strlen(absolutePathname)] = '/';
        absolutePathname[strlen(absolutePathname)] = '\0';
    }
    strcpy(current->cwd, absolutePathname);
    return 0;
}

int ufs_dup(int fd)
{
    int free_fd = 0;
    for (; free_fd < 128; free_fd++) {
        if (current->fds[free_fd] == -1) break;
    }
    if (free_fd == 128) return -1;
    current->fds[free_fd] = current->fds[fd];
    return free_fd;
}

/* ---------- UFS ---------- */

MODULE_DEF(vfs) = {
    .init = ufs_init,
    .write = ufs_write,
    .read = ufs_read,
    .close = ufs_close,
    .open = ufs_open,
    .lseek = ufs_lseek,
    .link = ufs_link,
    .unlink = ufs_unlink,
    .fstat = ufs_fstat,
    .mkdir = ufs_mkdir,
    .chdir = ufs_chdir,
    .dup = ufs_dup,
};
