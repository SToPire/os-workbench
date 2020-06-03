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

int main(int argc, char *argv[]) {
    struct stat fs;
    int fd = open(argv[1], O_RDONLY);
    fstat(fd, &fs);

    void* ImgPtr = mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    fat_header_t* fhp = (fat_header_t*)ImgPtr;
    void* FirstDataSector = ImgPtr + fhp->BPB_BytsPerSec * (fhp->BPB_RsvdSecCnt + fhp->BPB_NumFATs * fhp->BPB_FATSz32);
    printf("%u\n", fhp->BPB_FATSz32);
    
    close(fd);
    return 0;
}
