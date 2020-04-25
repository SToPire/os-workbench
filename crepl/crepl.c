#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <fcntl.h>

int FILECNT = 0;

typedef int (*WRAPPER)();

char funcs[100][4096];
int funcsCnt = 0;

int main(int argc, char* argv[])
{
    static char line[4096];
    while (1) {
        printf("crepl> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        // char Cname[32], Soname[32];
        // sprintf(Cname, "/tmp/crepl-%d.c", ++FILECNT);
        // sprintf(Soname, "/tmp/crepl-%d.so", FILECNT);
        // FILE* fp = fopen(Cname, "w+");
        char Cname[32] = "/tmp/crepl-XXXXXX";
        int fd = mkstemp(Cname);
        char Soname[40];
        sprintf(Soname, "%s.so", Cname);
        strcat(Cname, ".c");

        char wrapper[4096 + 64], wrapper_name[32];
        if (strncmp(line, "int", 3) == 0) {
            strcpy(funcs[funcsCnt++], line);
            for (int i = 0; i < funcsCnt; ++i) {
                //fputs(funcs[i], fp);
                write(fd, funcs[i], strlen(funcs[i]));
            }
        } else {
            for (int i = 0; i < funcsCnt; ++i) {
                //fputs(funcs[i], fp);
                write(fd, funcs[i], strlen(funcs[i]));
            }
            sprintf(wrapper_name, "__expr_wrapper_%d", FILECNT);
            sprintf(wrapper, "int __expr_wrapper_%d(){return %s;}", FILECNT, line);
            //fputs(wrapper, fp);
            write(fd, wrapper, strlen(wrapper));
        }

        close(fd);
        //fclose(fp);

        char* exec_argv[] = {"gcc", "-fPIC", "-shared", Cname, "-o", Soname, NULL};

        int pipe_fd[2];
        if (pipe(pipe_fd) < 0) {
            fprintf(stderr, "pipe create error\n");
            exit(EXIT_FAILURE);
        }

        __pid_t pid = fork();
        if (pid == 0) {
            //dup2(pipe_fd[1], STDERR_FILENO);
            execvp("gcc", exec_argv);
        } else {
            while (waitpid(pid, NULL, WNOHANG) != pid)
                ;

            int flag = fcntl(pipe_fd[0], F_GETFL);
            flag |= O_NONBLOCK;
            fcntl(pipe_fd[0], F_SETFL, flag);
            char ERR[16];
            if (read(pipe_fd[0], ERR, 1) == 1) {
                close(pipe_fd[0]);
                close(pipe_fd[1]);
                if (strncmp(line, "int", 3) == 0) --funcsCnt;
                continue;
            }

            void* handle = dlopen(Soname, RTLD_LAZY);
            if (!handle) {
                fprintf(stderr, "%s\n", dlerror());
                exit(EXIT_FAILURE);
            }

            if (strncmp(line, "int", 3) == 0) {
                ;
            } else {
                WRAPPER W;
                W = (WRAPPER)dlsym(handle, wrapper_name);
                printf("%d\n", W());
            }
            dlclose(handle);
        }
    }
}
