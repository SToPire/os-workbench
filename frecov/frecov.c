#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <ctype.h>

typedef __uint8_t u8;
typedef __uint16_t u16;
typedef __uint32_t u32;

typedef struct fat_header {
    u8 BS_jmpBoot[3];
    u8 BS_OEMName[8];
    u16 BPB_BytsPerSec;
    u8 BPB_SecPerClus;
    u16 BPB_RsvdSecCnt;
    u8 BPB_NumFATs;
    u16 BPB_RootEntCnt;
    u16 BPB_TotSec16;
    u8 BPB_Media;
    u16 BPB_FATSz16;
    u16 BPB_SecPerTrk;
    u16 BPB_NumHeads;
    u32 BPB_HiddSec;
    u32 BPB_TotSec32;
    u32 BPB_FATSz32;
    u16 BPB_ExtFlags;
    u16 BPB_FSVer;
    u32 BPB_RootClus;
    u16 BPB_FSInfo;
    u16 BPB_BkBootSec;
    u8 BPB_Reserved[12];
    u8 BS_DrvNum;
    u8 BS_Reserved1;
    u8 BS_BootSig;
    u32 BS_VolID;
    u8 BS_VolLab[11];
    u8 BS_FilSysTyp[8];

    u8 padding[420];
    u16 signature;
} __attribute__((packed)) fat_header_t;

typedef struct short_entry {
    u8 DIR_Name[8];
    u8 DIR_ExtName[3];
    u8 DIR_Attr;
    u8 DIR_NTRes;
    u8 DIR_CrtTimeTenth;
    u16 DIR_CrtTime;
    u16 DIR_CrtDate;
    u16 DIR_LstAccDate;
    u16 DIR_FstClusHI;
    u16 DIR_WrtTime;
    u16 DIR_WrtDate;
    u16 DIR_FstClusLO;
    u32 DIR_FileSize;
} __attribute__((packed)) sEntry_t;

typedef struct long_entry {
    u8 LDIR_Ord;
    u16 LDIR_Name1[5];
    u8 LDIR_Attr;
    u8 LDIR_Type;
    u8 LDIR_ChkSum;
    u16 LDIR_Name2[6];
    u16 LDIR_FstClusLO;
    u16 LDIR_Name3[2];
} __attribute__((packed)) lEntry_t;

int isDirEntryCluster(void* addr)
{
    int cnt = 0;
    for (char* i = (char*)addr; i <= (char*)addr + 4096 - 3; i++) {
        if ((*i == 'b' || *i == 'B') && (*(i + 1) == 'm' || *(i + 1) == 'M') && (*(i + 2) == 'p' || *(i + 2) == 'P')) cnt++;
    }
    if (cnt >= 5)
        return 1;
    else
        return 0;
}
#define NthClusterAddr(N) (((N - 2) * fhp->BPB_SecPerClus) * fhp->BPB_BytsPerSec + FirstDataSector)
int main(int argc, char* argv[])
{
    struct stat fs;
    int fd = open(argv[1], O_RDONLY);
    fstat(fd, &fs);

    void* ImgPtr = mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    fat_header_t* fhp = (fat_header_t*)ImgPtr;
    void* FirstDataCluster = ImgPtr + fhp->BPB_BytsPerSec * (fhp->BPB_RsvdSecCnt + fhp->BPB_NumFATs * fhp->BPB_FATSz32);
    if (fs.st_size > 100 * 1024 * 1024) FirstDataCluster += 4096 * 2;
    for (void* clusPtr = FirstDataCluster; clusPtr < ImgPtr + fs.st_size; clusPtr += 4096) {
        if (isDirEntryCluster(clusPtr)) {
            for (sEntry_t* left = clusPtr; (void*)left < clusPtr + 4096 - 32;) {
                if (left->DIR_Name[0] == 0xE5 || left->DIR_Name[0] == 0x00) {
                    ++left;
                    continue;
                }
                sEntry_t* right = left;
                while (right->DIR_Attr != 0x20 && (void*)right < clusPtr + 4096 - 32) ++right;
                char name[128];
                int nameptr = 0;

                int legalname = 0;
                if (left != right) {
                    for (lEntry_t* i = (lEntry_t*)(right - 1); i >= (lEntry_t*)left; i--) {
                        for (int j = 0; j < 5; j++) name[nameptr++] = (char)(i->LDIR_Name1[j]);
                        for (int j = 0; j < 6; j++) name[nameptr++] = (char)(i->LDIR_Name2[j]);
                        for (int j = 0; j < 2; j++) name[nameptr++] = (char)(i->LDIR_Name3[j]);
                    }
                    if (toupper(name[nameptr - 1] == right->DIR_ExtName[2] && tolower(name[0] == right->DIR_Name[0]))) legalname = 1;
                    name[nameptr++] = '\0';
                } else {
                    if (right->DIR_Name[0] == 0xE5 || right->DIR_Name[0] == 0x00) {
                        ++left;
                        continue;
                    } else {
                        assert(0);
                    }
                }
                left = right + 1;
                if (legalname) {
                    for (int i = 1; i <= 40; i++) putc('c', stdout);
                    putc(' ', stdout);
                    printf("%s\n", name);
                }
            }
        }
    }

    close(fd);
    return 0;
}
