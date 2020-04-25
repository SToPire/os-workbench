#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dlfcn.h>

#define ROOT /tmp/
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
        printf("%s", line);

        char prefix[] = "FILE-XXXXXX";
        int fd = mkstemp(prefix);
        printf("%s", prefix);
        char* exec_argv[] = {"gcc", "-fPIC", "-shared", "/tmp/tmp.c", "-o", "/tmp/tmp.so", NULL};
        __pid_t pid = fork();
        if (pid == 0) {
            execvp("gcc", exec_argv);
        } else {
            while (waitpid(pid, NULL, WNOHANG) != pid)
                ;
            void* handle = dlopen("/tmp/tmp.so", RTLD_LAZY);
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
