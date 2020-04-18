#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

typedef struct __node {
    char name[64];
    double t;
} node;

int main(int argc, char* argv[])
{
    char* exec_argv[argc + 2];
    exec_argv[0] = "strace";
    exec_argv[1] = "-T";
    for (int i = 1; i < argc; ++i)
        exec_argv[i + 1] = argv[i];
    exec_argv[argc + 1] = NULL;

    char* exec_envp[] = {
        NULL,
        NULL,
    };

    char* currenetPaths[32] = {NULL};

    extern char** environ;
    for (char** i = environ; *i != NULL; i++)
        if (strncmp(*i, "PATH=", 5) == 0) {
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
    if (pipe(pipe_fd) < 0) {
        printf("pipe create error\n");
        return -1;
    }

    __pid_t pid = fork();
    if (pid == 0) {  //children
        for (int i = 0; i < 32; i++)
            if (currenetPaths[i]) {
                dup2(pipe_fd[1], STDERR_FILENO);
                freopen("/dev/null", "w", stdout);
                char* newLoc = malloc(strlen(currenetPaths[i]) + 10);
                strcpy(newLoc, currenetPaths[i]);
                strcat(newLoc, "/strace");
                execve(newLoc, exec_argv, exec_envp);
            }
    } else {
        sleep(1);
        dup2(pipe_fd[0], STDIN_FILENO);
        //waitpid(pid,0,0);
        char s[512];
        node stat[128];
        for (int i = 0; i < 128; i++) {
            stat[i].t = 0.0;
            strcpy(stat[i].name, "");
        }
        memset(stat, 0, sizeof(stat));
        double tot = 0.0;
        while (fgets(s, 512, stdin)) {
            int i2 = strlen(s), i3 = strlen(s);
            while (s[i2] != '<' && i2 >= 0) --i2;
            while (s[i3] != '>' && i3 >= 0) --i3;
            if (i2 < 0 || i3 < 0) break;
            char ts[64];
            int tscnt = 0;
            for (int i = i2 + 1; i < i3; i++) ts[tscnt++] = s[i];
            ts[tscnt] = '\0';
            double t = strtod(ts, 0);

            int i1 = 0;
            char name[64];
            while (s[i1] != '(') ++i1;
            for (int i = 0; i < i1; ++i) name[i] = s[i];
            name[i1] = '\0';

            for (int i = 0; i < 127; ++i) {
                if (strcmp(stat[i].name, name) == 0) {
                    stat[i].t += t;
                    break;
                } else if (strcmp(stat[i].name,"")==0) {
                    strcpy(stat[i].name, name);
                    stat[i].t += t;
                    break;
                }
            }
            tot += t;

            if (strcmp(s, "+++ exited with 0 +++\n") == 0) break;
        }
        for (int i = 0; i < 128; i++) {
            if (strcmp(stat[i].name,"")!=0) {
                printf("%s:%f:%f\n", stat[i].name, stat[i].t,100*stat[i].t/tot);
            }
        }
        printf("HSHSHHSHSHSHS\n");
    }
    // perror(argv[0]);
    // exit(EXIT_FAILURE);
}
