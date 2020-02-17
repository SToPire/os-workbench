#include <stdio.h>
#include <assert.h>
#include<dirent.h>


#define PROC_BACE "/proc"

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);

  //DIR* opendir(PROC_BACE);

  return 0;
}
