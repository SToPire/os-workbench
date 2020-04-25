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
        printf("%d", funcsCnt);
        printf("crepl> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        char Cname[32], Soname[32];
        sprintf(Cname, "/tmp/crepl-%d.c", ++FILECNT);
        sprintf(Soname, "/tmp/crepl-%d.so", FILECNT);
        FILE* fp = fopen(Cname, "w+");

        char wrapper[4096 + 64], wrapper_name[32];
        char fun_name[4096];
        if (strncmp(line, "int", 3) == 0) {
            strcpy(fun_name, line + 4);
            int i = 0;
            for (; fun_name[i] != '(';++i)
                ;
            fun_name[i] = '\0';
            strcpy(funcs[funcsCnt++], line);

            for (int i = 0; i < funcsCnt; ++i) {
                fputs(funcs[i], fp);
            }
            sprintf(wrapper_name, "__expr_wrapper_%d", FILECNT);
            sprintf(wrapper, "int __expr_wrapper_%d(){return %s();}", FILECNT, fun_name);
            fputs(wrapper, fp);

        } else {
            for (int i = 0; i < funcsCnt; ++i) {
                fputs(funcs[i], fp);
            }
            sprintf(wrapper_name, "__expr_wrapper_%d", FILECNT);
            sprintf(wrapper, "int __expr_wrapper_%d(){return %s;}", FILECNT, line);
            fputs(wrapper, fp);
        }

        fclose(fp);

        char* exec_argv[] = {"gcc", "-w", "-fPIC", "-shared", Cname, "-o", Soname, NULL};

        int pipe_fd[2];
        if (pipe(pipe_fd) < 0) {
            printf("pipe create error\n");
            return -1;
        }

        __pid_t pid = fork();
        if (pid == 0) {
            dup2(pipe_fd[1], STDERR_FILENO);
            execvp("gcc", exec_argv);
        } else {
            while (waitpid(pid, NULL, WNOHANG) != pid)
                ;

            int flag = fcntl(pipe_fd[0],F_GETFL);
            flag |= O_NONBLOCK;
            fcntl(pipe_fd[0], F_SETFL, flag);
            char ERR[16];
            if (read(pipe_fd[0], ERR, 1) == 1){
                close(pipe_fd[0]);
                close(pipe_fd[1]);
                --funcsCnt;
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
