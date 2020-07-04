#include <user.h>
#include <stdio.h>
#include<stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

int main(int argc, char *argv[]) {
  int fd;
  uint8_t *disk;

  // TODO: argument parsing
  int IMG_SIZE = atoi(argv[1]) * 1024 * 1024;

  char* cwd = getcwd(NULL, 0);
  char* newwd = malloc(strlen(cwd) - strlen("/tools") + 1);
  strncpy(newwd, cwd, strlen(cwd) - strlen("/tools"));
  if (chdir(newwd) != 0) assert(0);
  printf("%d %s\n", IMG_SIZE, newwd);

  assert((fd = open(argv[2], O_RDWR)) > 0);
  assert((ftruncate(fd, IMG_SIZE)) == 0);
  assert((disk = mmap(NULL, IMG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) != (void *)-1);

  // TODO: mkfs

  munmap(disk, IMG_SIZE);
  close(fd);
}
