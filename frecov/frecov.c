#include<stdio.h>
#include<unistd.h>
#include<assert.h>

typedef __uint8_t u8;
typedef __uint16_t u16;
typedef __uint32_t u32;

struct fat_header {
    u8 BS_jmpBoot[3];
    u8 BS_OEMName[8];
    u32 BPB_BytsPerSec : 16;
    u32 BPB_SecPerClus : 8;
    
    u8 padding[420];
    u16 signature;
} __attribute__((packed));

int main(int argc, char *argv[]) {
    assert(sizeof(struct fat_header) == 512);
}
