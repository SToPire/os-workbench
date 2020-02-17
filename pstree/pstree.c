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
              char BUF[128*8],name[32],unused[32];
              fread(BUF, 1, 128, file);
              sscanf(BUF, "Name:%[^' ',^\n]\nUmask:%s\nState:%[^\n]\nTgid:%d\nNgid:%s\nPid:%d\nPPid:%d\n", name, unused, unused, &Tgid, unused, &Pid, &PPid);
              printf("%s %d %d %d\n", name, Tgid, Pid, PPid);
              printf("%s\n\n", BUF);
          }
      }
  }

  return 0;
}
