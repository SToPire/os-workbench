#include <stdio.h>
#include <assert.h>
#include<sys/types.h>
#include<dirent.h>
#include<stdlib.h>

#define PROC_BACE "/proc"

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);

  DIR* dir = opendir(PROC_BACE);
  struct dirent* dir_entry;
  while ((dir_entry = readdir(dir)) != NULL) {
      pid_t pid;
      if((pid = (pid_t)atoi(dir_entry->d_name)) != 0){
          char path[32];
          sprintf(path, "%s/%d/status", PROC_BACE, pid);
          FILE* file;
          if((file = fopen(path,"r")) != NULL){
              int Tgid, Pid, PPid;
              char BUF[128*8],name[32],unused[32],unused1[32],unused2[32];
              fread(BUF, 1, 128, file);
              sscanf(BUF, "Name:%s\nUmask:%s\nState:%s\nTgid:%d\nNgid:%s\nPid:%d\nPPid:%d\n", name,unused,unused1,&Tgid,unused2,&Pid,&PPid);
              //printf("%s:%d %d %d\n", name, Tgid, Pid, PPid);
              printf("%s %s %s %s\n", name, unused,unused1,unused2);
              printf("%s\n\n", BUF);
          }
      }
  }

  return 0;
}
