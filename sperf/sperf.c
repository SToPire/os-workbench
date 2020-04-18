#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>

int main(int argc, char* argv[])
{
    char* exec_argv[] = {"strace",NULL,NULL,};
    exec_argv[1] = argv[1];
    char *exec_envp[] = { NULL, NULL, };

    char* currenetPaths[20];

    extern char** environ;
    for (char ** i = environ; *i != NULL; i++)
        if (strncmp(*i, "PATH=", 5) == 0){
            exec_envp[0] = *i;
            char* tmp = malloc(sizeof(*i));
            strcpy(tmp, *i + 5);
            strtok(tmp, ":");
            char* s;
            int ii = 0;
            while ((currenetPaths[ii++] = strtok(NULL, ":")))
                ;
            break;
        }

    // for (int ii = 0; ii < 20;ii++){
    //     if (currenetPaths[ii]) printf("%s\n", currenetPaths[ii]);
    // }
    printf("%s\n", exec_envp[0]);
    execve("/usr/bin/strace", exec_argv, exec_envp);
    // perror(argv[0]);
    // exit(EXIT_FAILURE);
}
