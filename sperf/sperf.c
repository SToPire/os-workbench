#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

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

    char* currenetPaths[32] = {NULL};
    char* exec_envp[128];
    exec_envp[0] = "2>/dev/null";

    extern char** environ;
    int envCnt = 1;
    for (char** i = environ; *i != NULL; i++) {
        exec_envp[envCnt++] = *i;
        if (strncmp(*i, "PATH=", 5) == 0) {
            //exec_envp[0] = *i;
            char* tmp = malloc(strlen(*i));
            strcpy(tmp, *i + 5);
            strtok(tmp, ":");
            int ii = 0;
            while ((currenetPaths[ii++] = strtok(NULL, ":")))
                ;
            break;
        }
    }
    exec_envp[envCnt] = NULL;
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
        int flag = fcntl(STDIN_FILENO, F_GETFL);
        flag |= O_NONBLOCK;
        fcntl(STDIN_FILENO, F_SETFL, flag);

        char s[512];
        node stat[128];
        double tot = 0.0;

        int cnt = 0;

        while (1) {
            int status;
            for (int i = 0; i < 128; i++) {
                stat[i].t = 0.0;
                strcpy(stat[i].name, "");
            }
            tot = 0.0;
            int ret;
            //if ((ret = read(STDIN_FILENO, s, sizeof(s))) <= 0 && waitpid(pid, &status, WNOHANG) == pid) break;
            if (fgets(s, sizeof(s), stdin) == NULL && waitpid(pid, &status, WNOHANG) == pid) break;
            //printf("%s\n", s);

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
                } else if (strcmp(stat[i].name, "") == 0) {
                    strcpy(stat[i].name, name);
                    stat[i].t += t;
                    break;
                }
            }
            tot += t;
            //if (++cnt == 100000) break;
            //if (strcmp(s, "+++ exited with 0 +++\n") == 0) break;

            //     puts("==========================");
            //     for (int i = 1; i <= 80; i++) putc(0, stdout);
            //     fflush(stdout);
            //     if (waitpid(pid, &status, WNOHANG) == pid) break;
        }
        for (int i = 0; i < 128; i++) {
            if (strcmp(stat[i].name, "") != 0) {
                printf("%s(%.0f%%)\n", stat[i].name, 100 * stat[i].t / tot);
            }
        }
    }
}
