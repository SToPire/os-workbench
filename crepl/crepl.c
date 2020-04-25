#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dlfcn.h>

int FILECNT = 0;

typedef int (*WRAPPER)();
int main(int argc, char* argv[])
{
    static char line[4096];
    while (1) {
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
        if (strncmp(line, "int", 3) == 0) {
            fputs(line, fp);
        } else {
            sprintf(wrapper_name, "__expr_wrapper_%d", FILECNT);
            sprintf(wrapper, "int __expr_wrapper_%d(){return %s;}", FILECNT, line);
            fputs(wrapper, fp);
        }

        fclose(fp);

        char* exec_argv[] = {"gcc", "-fPIC", "-shared", Cname, "-o", Soname, NULL};
        __pid_t pid = fork();
        if (pid == 0) {
            execvp("gcc", exec_argv);
        } else {
            while (waitpid(pid, NULL, WNOHANG) != pid)
                ;
            void* handle = dlopen(Soname, RTLD_LAZY);
            if (!handle) {
                fprintf(stderr, "%s\n", dlerror());
                exit(EXIT_FAILURE);
            }
            if (strncmp(line, "int", 3) == 0) {
            } else {
                WRAPPER W;
                W = (WRAPPER)dlsym(handle, wrapper_name);
                printf("%d\n", W());
            }
        }
    }
}
