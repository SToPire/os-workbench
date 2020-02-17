#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

// #define MAX_PROC 10000
// typedef struct {
//     Node* next;

//     char name[32];
//     pid_t pid;
//     Node* children;
// } Node;

// Node* root;
// Node* NodeList[MAX_PROC];
// int NodeListCnt = 0;
// Node* new_node(const char* name, pid_t pid)
// {
//     Node* ptr = malloc(sizeof(Node));
//     strcpy(ptr->name, name);
//     ptr->pid = pid;

//     Node* head = malloc(sizeof(Node));
//     Node* tail = malloc(sizeof(Node));
//     strcpy(head->name, "");
//     strcpy(tail->name, "");
//     head->pid = tail->pid = -1;

//     ptr->children = head;
//     head->next = tail;

//     NodeList[NodeListCnt++] = ptr;
//     return ptr;
// }

// Node* find_node(pid_t p){
//     for (int i = 0; i < NodeListCnt;++i)
//         if (NodeList[i]->pid == p) return NodeList[i];
//     return NULL;
// }
// void add_proc(pid_t parent, pid_t child, const char* child_name)
// {
//     if(parent == 0){
//         root = new_node(child_name, child);
//     }
//     Node* ptr = find_proc(parent);
//     if (!ptr) ptr = new_node(NULL, parent);
// }

int main(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++) {
        assert(argv[i]);
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]);



    DIR* dir = opendir("/proc");
    struct dirent* dir_entry;
    while ((dir_entry = readdir(dir)) != NULL) {
        pid_t pid;
        if ((pid = (pid_t)atoi(dir_entry->d_name)) != 0) {
            char path[32];
            sprintf(path, "/proc/%d/status", pid);
            FILE* file;
            if ((file = fopen(path, "r")) != NULL) {
                pid_t Tgid, Pid, PPid, parent;
                char BUF[128 * 8], name[32];
                fread(BUF, 1, 128, file);
                sscanf(BUF, "Name: %[^\n]\nUmask:%*s\nState:%*[^\n]\nTgid:%d\nNgid:%*s\nPid:%d\nPPid:%d\n", name, &Tgid, &Pid, &PPid);
               // printf("%s %d %d %d\n", name, Tgid, Pid, PPid);
               // printf("%s\n\n", BUF);
                if (Tgid == Pid) 
                    parent = PPid;
                else
                    parent = Tgid;
                assert(parent < pid);
            }
        }
    }

    return 0;
}
