#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    extern char ** environ;
    char** i;
    for (i = environ; *i != NULL; i++) printf("%s\n", *i);

    char* exec_argv[] = {
        "strace",
        NULL,
        NULL,
    };
    char *exec_envp[] = { "PATH=/bin", NULL, };
    exec_argv[1] = argv[1];
    //execve("strace",          exec_argv, exec_envp);
    //execve("/bin/strace",     exec_argv, exec_envp);
    execve("/usr/bin/strace", exec_argv, exec_envp);
    // perror(argv[0]);
    // exit(EXIT_FAILURE);
}
