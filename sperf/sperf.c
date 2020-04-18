#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>

int main(int argc, char *argv[]) {
    char* exec_argv[] = {"strace",NULL,NULL,};
    char *exec_envp[] = { NULL, NULL, };
    extern char ** environ;
    for (char ** i = environ; *i != NULL; i++)
        if (strncmp(*i, "PATH=", 5) == 0)
            exec_envp[0] = *i;
    exec_argv[1] = argv[1];

    execve("strace", exec_argv, exec_envp);
    // perror(argv[0]);
    // exit(EXIT_FAILURE);
}
