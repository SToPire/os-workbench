#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>

int main(int argc, char *argv[]) {
    char *exec_envp[] = { NULL, NULL, };
    extern char ** environ;
    for (char ** i = environ; *i != NULL; i++){
        if (strncmp(*i, "PATH=", 5) == 0){
            exec_envp[0] = *i;
            // strtok(*i + 5, ":");
            // char* s;
            // while ((s = strtok(NULL, ":"))) printf("%s\n", s);
        }
    }

    char* exec_argv[] = {
        "strace",
        NULL,
        NULL,
    };
    exec_argv[1] = argv[1];

    execve("/usr/bin/strace", exec_argv, exec_envp);
    // perror(argv[0]);
    // exit(EXIT_FAILURE);
}
