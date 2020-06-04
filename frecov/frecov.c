#include<stdio.h>
#include<unistd.h>
#include<assert.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/fcntl.h>
#include<sys/mman.h>

typedef __uint8_t u8;
typedef __uint16_t u16;
typedef __uint32_t u32;

typedef  struct fat_header {
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

typedef struct short_entry{
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

#define NthClusterAddr(N) (((N - 2) * fhp->BPB_SecPerClus) * fhp->BPB_BytsPerSec + FirstDataSector)
int main(int argc, char *argv[]) {
    struct stat fs;
    int fd = open(argv[1], O_RDONLY);
    fstat(fd, &fs);

    void* ImgPtr = mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    fat_header_t* fhp = (fat_header_t*)ImgPtr;
    void* FirstDataSector = ImgPtr + fhp->BPB_BytsPerSec * (fhp->BPB_RsvdSecCnt + fhp->BPB_NumFATs * fhp->BPB_FATSz32);

    sEntry_t* test = (sEntry_t*)FirstDataSector;
    //test = (sEntry_t*)((void*)test + 8 * 512);
    //test++;
    printf("attr: %x ", test->DIR_Attr);
    for (int i = 0; i < 8; i++)
        printf("%c", test->DIR_Name[i]);
    u32 FirstCluster = (u32)(test->DIR_FstClusHI) << 16 | (u32)(test->DIR_FstClusLO);

    sEntry_t* t2 = (sEntry_t*)NthClusterAddr(FirstCluster);
    t2 += 3;
    printf("attr: %x ", t2->DIR_Attr);
    for (int i = 0; i < 8; i++)
        printf("%c", t2->DIR_Name[i]);

    close(fd);
    return 0;
}
