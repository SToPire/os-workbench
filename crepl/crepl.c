#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include<unistd.h>
int main(int argc, char *argv[]) {
    // char path[256];
    // sprintf(path, "PATH=%s", getenv("PATH"));
    // char* exec_envp[] = {NULL, NULL};
    // exec_envp[0] = path;

    char* exec_argc[] = {"-fPIC", "-shared", "/tmp/tmp.c", "-o", "/tmp/tmp.o"};
    __pid_t pid = fork();
    if(pid==0){
        execvp("gcc", exec_argc);
    } else {
        static char line[4096];
        while (1) {
            printf("crepl> ");
            fflush(stdout);
            if (!fgets(line, sizeof(line), stdin)) {
                break;
            }
            printf("%s", line);
            printf("Got %zu chars.\n", strlen(line));  // WTF?
        }
    }
}
