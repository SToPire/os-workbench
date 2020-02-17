#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

struct Node {
    char name[32];
    pid_t pid;
    struct Node* children;
} head, tail;

void add_proc(pid_t parent, pid_t child)
{

}

int main(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++) {
        assert(argv[i]);
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]);

    strcpy(head.name, "00000");
    strcpy(tail.name, "zzzzz");
    head.pid = 0;
    tail.pid = __INT_MAX__;
    head.children = &tail;
    printf("%d\n", head.children->pid);

    DIR* dir = opendir("/proc");
    struct dirent* dir_entry;
    while ((dir_entry = readdir(dir)) != NULL) {
        pid_t pid;
        if ((pid = (pid_t)atoi(dir_entry->d_name)) != 0) {
            char path[32];
            sprintf(path, "/proc/%d/status", pid);
            FILE* file;
            if ((file = fopen(path, "r")) != NULL) {
                pid_t Tgid, Pid, PPid;
                char BUF[128 * 8], name[32];
                fread(BUF, 1, 128, file);
                sscanf(BUF, "Name: %[^\n]\nUmask:%*s\nState:%*[^\n]\nTgid:%d\nNgid:%*s\nPid:%d\nPPid:%d\n", name, &Tgid, &Pid, &PPid);
               // printf("%s %d %d %d\n", name, Tgid, Pid, PPid);
               // printf("%s\n\n", BUF);
            }
        }
    }

    return 0;
}
