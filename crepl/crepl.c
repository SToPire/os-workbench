#include <stdio.h>
#include <string.h>
#include<stdlib.h>

int main(int argc, char *argv[]) {
    // char path[256];
    // sprintf(path, "PATH=%s", getenv("PATH"));
    // char* exec_envp[] = {NULL, NULL};
    // exec_envp[0] = path;

    __pid_t pid = fork();
    if(pid==0){
        execvp("gcc", "-fPIC -shared /tmp/tmp.c -o /tmp/tmp.o ");
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
