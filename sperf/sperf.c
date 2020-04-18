#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<string.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{
    char* exec_argv[argc+2];
    exec_argv[0] = "strace";
    exec_argv[1] = "-T";
    for (int i = 1; i < argc;++i)
        exec_argv[i + 1] = argv[i];
    exec_argv[argc+1]=NULL;

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
        return -1;
    }

        __pid_t pid = fork();
    if(pid==0){ //children
        for (int i = 0; i < 32;i++)
            if(currenetPaths[i]){
                dup2(pipe_fd[1], STDERR_FILENO);
                char* newLoc = malloc(strlen(currenetPaths[i]) + 10);
                strcpy(newLoc, currenetPaths[i]);
                strcat(newLoc, "/strace");
                execve(newLoc, exec_argv, exec_envp);
            }
    }else{
        sleep(1);
        dup2(pipe_fd[0], STDIN_FILENO);
        //waitpid(pid,0,0);
        //char s[512];
        //while (fgets(s,512,stdin)){
            //printf("%s\n", s);
        //}
        printf("HSHSHHSHSHSHS\n");
    }
    // perror(argv[0]);
    // exit(EXIT_FAILURE);
}
