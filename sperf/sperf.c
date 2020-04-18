#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>

int main(int argc, char *argv[]) {
    extern char ** environ;
    char** i;
    for (i = environ; *i != NULL; i++){
        if (strncmp(*i, "PATH=", 5) == 0) printf("%s\n", *i);
    }

    char* exec_argv[] = {
        "strace",
        NULL,
        NULL,
    };
    char *exec_envp[] = { "PATH=/bin", NULL, };
    exec_argv[1] = argv[1];

    execve("/usr/bin/strace", exec_argv, exec_envp);
    // perror(argv[0]);
    // exit(EXIT_FAILURE);
}
