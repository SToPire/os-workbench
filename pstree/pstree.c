#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROC 10000

int SHOWPID = 0;
int NUMERICSORT = 0;

struct Node {
    struct Node* next;

    char name[64];
    pid_t pid;
    struct Node* children;
} *root;

struct Node* NodeList[MAX_PROC];
int NodeListCnt = 0;
struct Node* new_node(const char* name, pid_t pid)
{
    struct Node* ptr = malloc(sizeof(struct Node));
    strcpy(ptr->name, name);
    ptr->pid = pid;

    struct Node* head = malloc(sizeof(struct Node));
    struct Node* tail = malloc(sizeof(struct Node));
    strcpy(head->name, "");
    strcpy(tail->name, "");
    head->pid = -1;
    tail->pid = __INT_MAX__;

    ptr->children = head;
    head->next = tail;

    NodeList[NodeListCnt++] = ptr;
    return ptr;
}

// struct Node* find_node(pid_t p){
//     for (int i = 0; i < NodeListCnt;++i)
//         if (NodeList[i]->pid == p) return NodeList[i];
//     return NULL;
// }

struct Node* find_node(struct Node* cur, pid_t pid)
{
    if (cur->pid == pid) return cur;
    struct Node* ret;
    if ((ret = find_node(cur->children,pid)) != NULL)
        return ret;
    else
        return find_node(cur->next,pid);
}

void add_node(pid_t parent, pid_t child, const char* child_name)
{

    struct Node* father = find_node(root, parent);
    if (!father) assert(0);

    struct Node* cld = new_node(child_name, child);

    struct Node* pre = father->children;
    struct Node* cur = pre->next;
    while(cur->pid<child){
        cur = cur->next;
        pre = pre->next;
    }
    cld->next = cur;
    pre->next = cld;
}

void Print(struct Node* cur, int level)
{
    for (int i = 1; i < level; i++) printf("\t");
    if(SHOWPID)
        printf("%s(%d)\n", cur->name,cur->pid);
    else
        printf("%s\n", cur->name);
    for (struct Node* it = cur->children->next; it->pid != __INT_MAX__;it = it->next)
        Print(it, level + 1);
}

int main(int argc, char* argv[])
{
    for (int i = 0; i < argc; i++) {
        assert(argv[i]);
        if(i!=0){
            if(strcmp(argv[i],"-V") == 0 || strcmp(argv[i],"--version")==0){
                fprintf(stderr, "pstree\nCopyright (C) 2020 Yifan Zhao\n\nFree Software\n");
                return 0;
            } else if (strcmp(argv[i], "-p") == 0|| strcmp(argv[i], "--show-pids") == 0) {
                SHOWPID = 1;
            } else if (strcmp(argv[i],"-n")==0 || strcmp(argv[i],"--numeric-sort") == 0){
                NUMERICSORT = 1;
            }else{
                fprintf(stderr, "Illegal parameter!\n");
                return -1;
            }
        }
    }
    assert(!argv[argc]);

    root = new_node("", 0);

    DIR* dir = opendir("/proc");
    struct dirent* dir_entry;
    while ((dir_entry = readdir(dir)) != NULL) {
        pid_t pid;
        if ((pid = (pid_t)atoi(dir_entry->d_name)) != 0) {
            char path[64];
            sprintf(path, "/proc/%d/status", pid);
            FILE* file;
            if ((file = fopen(path, "r")) != NULL) {
                pid_t Tgid, Pid, PPid, parent;
                char BUF[128 * 8], name[64];
                fread(BUF, 1, 128, file);
                sscanf(BUF, "Name: %[^\n]\nUmask:%*s\nState:%*[^\n]\nTgid:%d\nNgid:%*s\nPid:%d\nPPid:%d\n", name, &Tgid, &Pid, &PPid);
                if (Tgid == Pid) 
                    parent = PPid;
                else
                    parent = Tgid;
                assert(parent < pid);
                add_node(parent, Pid, name);
            }
        }
    }

    Print(root, 0);
    return 0;
}
