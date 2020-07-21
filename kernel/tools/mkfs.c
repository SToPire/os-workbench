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
#define LNKFILE_NUM 128

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

typedef struct _linkFileEntry {
    __ino_t ino;
    uint32_t myino;
} linkFileEntry_t;

superblock_t sb;
uint8_t* fs_head;

linkFileEntry_t linkFile[LNKFILE_NUM];
int linkFileCnt;

void addFAT(uint32_t from, uint32_t to)
{
    memcpy(fs_head + sb.fat_head + sizeof(uint32_t) * from, (void*)(&to), sizeof(uint32_t));
}

void traverse(char* pathname, uint32_t parentino)
{
    dinode_t dirInode;
    memset(&dirInode, 0, sizeof(dirInode));
    dirInode.stat.id = sb.fst_free_inode++;
    dirInode.stat.type = T_DIR;
    dirInode.stat.size = 0;
    dirInode.firstBlock = sb.fst_free_data_blk++;
    dirInode.refCnt = 1;

    uint32_t lstBlk = dirInode.firstBlock;

    DIR* dir = opendir(pathname);
    struct dirent* dir_entry;
    while ((dir_entry = readdir(dir)) != NULL) {
        dirInode.stat.size += sizeof(struct ufs_dirent);
        uint32_t newBlk = sb.fst_free_data_blk++;
        addFAT(lstBlk, newBlk);
        struct ufs_dirent d;
        memset(&d, 0, sizeof(struct ufs_dirent));
        if (strcmp(dir_entry->d_name, ".") == 0) {
            d.inode = dirInode.stat.id;
            strcpy(d.name, ".");
            memcpy(fs_head + sb.data_head + sb.blk_size * lstBlk, (void*)(&d), sizeof(struct ufs_dirent));
            lstBlk = newBlk;
            continue;
        } else if (strcmp(dir_entry->d_name, "..") == 0) {
            d.inode = parentino;
            strcpy(d.name, "..");
            memcpy(fs_head + sb.data_head + sb.blk_size * lstBlk, (void*)(&d), sizeof(struct ufs_dirent));
            lstBlk = newBlk;
            continue;
        } else if (dir_entry->d_type == 8) {  // file
            d.inode = sb.fst_free_inode;
            strcpy(d.name, dir_entry->d_name);

            char fullPath[512];
            if (strcmp(pathname, "/") == 0) {
                sprintf(fullPath, "/%s", dir_entry->d_name);
            } else if (pathname[strlen(pathname) - 1] != '/'){
                sprintf(fullPath, "%s/%s", pathname, dir_entry->d_name);
            }else{
                sprintf(fullPath, "%s%s", pathname, dir_entry->d_name);
            }
            int fd = open(fullPath, O_RDWR);
            assert(fd > 0);
            struct stat statbuf;
            fstat(fd, &statbuf);

            if (1 != statbuf.st_nlink) {
                char linkFileFlag = 0;
                for (int i = 0; i < LNKFILE_NUM; i++)
                    if (linkFile[i].ino == statbuf.st_ino) {
                        linkFileFlag = 1;
                        d.inode = linkFile[i].myino;
                        memcpy(fs_head + sb.data_head + sb.blk_size * lstBlk, (void*)(&d), sizeof(struct ufs_dirent));
                        lstBlk = newBlk;
                        break;
                    }
                if (linkFileFlag)
                    continue;
                else {
                    linkFile[linkFileCnt].ino = statbuf.st_ino;
                    linkFile[linkFileCnt++].myino = d.inode;
                }
            }

            ++sb.fst_free_inode;
            dinode_t newDinode;
            memset(&newDinode, 0, sizeof(newDinode));
            newDinode.stat.id = d.inode;
            newDinode.stat.type = T_FILE;
            newDinode.stat.size = statbuf.st_size;
            newDinode.refCnt = statbuf.st_nlink;
            newDinode.firstBlock = sb.fst_free_data_blk++;
            memcpy(fs_head + sb.inode_head + sb.inode_size * newDinode.stat.id, (void*)(&newDinode), sizeof(dinode_t));

            uint32_t curBlk = newDinode.firstBlock;
            uint32_t remain = newDinode.stat.size;
            char* buf = malloc(sb.blk_size);
            while (remain > 0) {
                uint32_t nxtBlk = sb.fst_free_data_blk++;
                addFAT(curBlk, nxtBlk);
                memset(buf, 0, sb.blk_size);
                uint32_t curSize = (sb.blk_size > remain) ? remain : sb.blk_size;
                if (read(fd, buf, curSize) <= 0) assert(-1);
                memcpy(fs_head + sb.data_head + sb.blk_size * curBlk, buf, curSize);
                curBlk = nxtBlk;
                remain -= curSize;
            }
            close(fd);
            memcpy(fs_head + sb.data_head + sb.blk_size * lstBlk, (void*)(&d), sizeof(struct ufs_dirent));
            lstBlk = newBlk;
        } else if (dir_entry->d_type == 4) {  // dir
            d.inode = sb.fst_free_inode++;
            printf("%d\n", d.inode);
            strcpy(d.name, dir_entry->d_name);
            memcpy(fs_head + sb.data_head + sb.blk_size * lstBlk, (void*)(&d), sizeof(struct ufs_dirent));
            lstBlk = newBlk;

            // dinode_t newDinode;
            // memset(&newDinode, 0, sizeof(newDinode));
            // newDinode.stat.id = d.inode;
            // newDinode.stat.size = 0;
            // newDinode.stat.type = T_DIR;
            // newDinode.refCnt = 1;
            // newDinode.firstBlock = sb.fst_free_data_blk++;
            // memcpy(fs_head + sb.inode_head + sb.inode_size * newDinode.stat.id, (void*)(&newDinode), sizeof(dinode_t));

            char fullPath[512];
            if (strcmp(pathname, "/") == 0) {
                sprintf(fullPath, "/%s", dir_entry->d_name);
            } else if (pathname[strlen(pathname) - 1] != '/') {
                sprintf(fullPath, "%s/%s", pathname, dir_entry->d_name);
            } else {
                sprintf(fullPath, "%s%s", pathname, dir_entry->d_name);
            }
            traverse(fullPath, d.inode);
        }
    }
    memcpy(fs_head + sb.inode_head + sb.inode_size * dirInode.stat.id, (void*)(&dirInode), sizeof(dinode_t));
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

    memset(linkFile, -1, sizeof(linkFile));
    linkFileCnt = 0;
    traverse(argv[3], 0);
    memcpy(fs_head, (void*)(&sb), sizeof(sb));

    munmap(disk, IMG_SIZE);
    close(fd);
}
