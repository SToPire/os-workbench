#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>

int main(int argc, char* argv[])
{
    char* exec_argv[] = {"strace","-T",NULL,NULL,};
    exec_argv[2] = argv[1];
    char *exec_envp[] = { NULL, NULL, };

    char* currenetPaths[32]={NULL};

    extern char** environ;
    for (char ** i = environ; *i != NULL; i++)
        if (strncmp(*i, "PATH=", 5) == 0){
            exec_envp[0] = *i;
            char* tmp = malloc(strlen(*i));
            strcpy(tmp, *i + 5);
            strtok(tmp, ":");
            int ii = 0;
            while ((currenetPaths[ii++] = strtok(NULL, ":")))
                ;
            break;
        }

    int pipe_fd[2];
    if (pipe(pipe_fd)<0){
        printf("pipe create error\n");
        assert(0);
    }

        __pid_t pid = fork();
    if(pid==0){ //children
        for (int i = 0; i < 32;i++)
            if(currenetPaths[i]){
                dup2(pipe_fd[1], 2);
                char* newLoc = malloc(strlen(currenetPaths[i]) + 10);
                strcpy(newLoc, currenetPaths[i]);
                strcat(newLoc, "/strace");
                execve(newLoc, exec_argv, exec_envp);
            }
    }else{
        printf("HSHSHHSHSHSHS\n");
    }
    // perror(argv[0]);
    // exit(EXIT_FAILURE);
}
