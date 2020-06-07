#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    u8 LDIR_Name1[5][2];
    u8 LDIR_Attr;
    u8 LDIR_Type;
    u8 LDIR_ChkSum;
    u8 LDIR_Name2[6][2];
    u16 LDIR_FstClusLO;
    u8 LDIR_Name3[2][2];
} __attribute__((packed)) lEntry_t;

typedef struct bmp_header {
    u8 type[2];
    u32 size;
    u8 meaningless1[4];
    u32 offset;
    u8 meaningless2[4];
    u32 width;
    u32 height;
    u8 meaningless3[28];
} __attribute((packed)) bmp_header_t;

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
int tcnt = 0;
int isLegalChar(char c)
{
    if (c == 0x2E || c == 0x5F) return 1;
    if (c >= 0x30 && c <= 0x39) return 1;
    if (c >= 0x41 && c <= 0x5A) return 1;
    if (c >= 0x61 && c <= 0x7A) return 1;
    return 0;
}

#define NthClusterAddr(N) (((N - 2) * fhp->BPB_SecPerClus) * fhp->BPB_BytsPerSec + FirstDataCluster)
#define BytesPerCluster (fhp->BPB_BytsPerSec * fhp->BPB_SecPerClus)
#define TotalClusterCnt (fs.st_size / BytesPerCluster)
int main(int argc, char* argv[])
{
    struct stat fs;
    int fd = open(argv[1], O_RDONLY);
    fstat(fd, &fs);

    void* ImgPtr = mmap(NULL, fs.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    fat_header_t* fhp = (fat_header_t*)ImgPtr;
    void* FirstDataCluster = ImgPtr + fhp->BPB_BytsPerSec * (fhp->BPB_RsvdSecCnt + fhp->BPB_NumFATs * fhp->BPB_FATSz32);
    for (void* clusPtr = FirstDataCluster; clusPtr < ImgPtr + fs.st_size; clusPtr += BytesPerCluster) {
        if (isDirEntryCluster(clusPtr)) {
            for (sEntry_t* left = clusPtr; (void*)left < clusPtr + BytesPerCluster;) {
                if (left->DIR_Name[0] == 0xE5 || left->DIR_Name[0] == 0x00) {
                    ++left;
                    continue;
                }
                sEntry_t* right = left;
                while (right->DIR_Attr != 0x20 && (void*)right < clusPtr + BytesPerCluster) ++right;
                char name[128];
                int nameptr = 0;

                int legalname = 0;
                if (left != right) {
                    for (lEntry_t* i = (lEntry_t*)(right - 1); i >= (lEntry_t*)left; i--) {
                        for (int j = 0; j < 5; j++)
                            if (isLegalChar((i->LDIR_Name1[j][0]))) name[nameptr++] = (i->LDIR_Name1[j][0]);
                        for (int j = 0; j < 6; j++)
                            if (isLegalChar((i->LDIR_Name2[j][0]))) name[nameptr++] = (i->LDIR_Name2[j][0]);
                        for (int j = 0; j < 2; j++)
                            if (isLegalChar((i->LDIR_Name3[j][0]))) name[nameptr++] = (i->LDIR_Name3[j][0]);
                    }
                    if (toupper(name[nameptr - 1]) == right->DIR_ExtName[2] && toupper(name[0]) == right->DIR_Name[0]) legalname = 1;
                    name[nameptr++] = '\0';
                } else {
                    ++left;
                    continue;
                }
                if (legalname) {
                    if (right->DIR_Attr == 0x20) {
                        u32 NumCluster = (right->DIR_FstClusHI << 16) | right->DIR_FstClusLO;
                        //if (NumCluster >= 0 && NumCluster <= TotalClusterCnt) {
                        if (NumCluster == 99) {
                            bmp_header_t* bmph = (bmp_header_t*)NthClusterAddr(NumCluster);
                            if (bmph->type[0] != 0x42 || bmph->type[1] != 0x4d) continue;

                            //char t[32];
                            //sprintf(t, "/tmp/%d.bmp", ++tcnt);
                            //FILE* fp = fopen(t, "w");
                            int bmpoffset = bmph->offset, bmpsize = bmph->size, width = bmph->width, height = bmph->height;
                            FILE* fp = fopen("/tmp/frecov-tmpfile", "w");
                            fwrite((void*)bmph, bmpoffset, 1, fp);
                            void* ptr1 = (void*)bmph + bmpoffset;
                            void* ptr2 = ptr1 +2* BytesPerCluster;
                            bmpsize -= bmpoffset;
                            //while (bmpsize) {
                            char tmpbuf[2 * BytesPerCluster];
                            memcpy(tmpbuf, ptr1, BytesPerCluster);
                            memcpy(tmpbuf + BytesPerCluster, ptr2, BytesPerCluster);
                            int i = 0;
                            for (; i + width * 3 < 2 * BytesPerCluster; i++) {
                                if (abs(tmpbuf[i] - tmpbuf[i + width * 3]) < 30) tcnt++;
                            }
                            printf("%d %d\n", tcnt, i);
                            // }

                            fclose(fp);

                            char buf[41];
                            fp = popen("sha1sum /tmp/frecov-tmpfile", "r");
                            fscanf(fp, "%s", buf);  // Get it!
                            pclose(fp);

                            printf("%s %s\n", buf, name);
                        } else {
                            ++left;
                            continue;
                        }
                    } else {
                        ++left;
                        continue;
                    }
                }
                left = right + 1;
            }
        }
    }

    close(fd);
    return 0;
}
