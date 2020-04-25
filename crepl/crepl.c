#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dlfcn.h>


int FILECNT = 0;

typedef int (*FUN)();
int main(int argc, char* argv[])
{
    static char line[4096];
    while (1) {
        printf("crepl> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        char Cname[32],Soname[32];
        sprintf(Cname, "/tmp/crepl-%d.c", ++FILECNT);
        sprintf(Soname, "/tmp/crepl-%d.so", FILECNT);
        FILE* fp = fopen(Cname, "r");
        //fputs("int f(){return 233;}", fp);

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
            FUN F;
            F = (FUN)dlsym(handle, "f");
            printf("output:%d\n", F());

            printf("Got %zu chars.\n", strlen(line));  // WTF?
        }
    }
}
