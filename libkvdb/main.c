#include "kvdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
int main()
{
    __pid_t pid = fork();
    if (pid == 0) {
        struct kvdb* db = kvdb_open("miao.db");
        for (int i = 1; i <= 2;i++)
            kvdb_put(db, "aaa", "bbb");
        for (int i = 1; i <= 2; i++)
            kvdb_put(db, "aaa", "bcb");
        free(db);
        kvdb_close(db);
    } else {
        while (waitpid(pid, NULL, WNOHANG) != pid)
            ;
        struct kvdb* db = kvdb_open("miao.db");
        char* s1 = kvdb_get(db, "aaa");
        free(db);
        kvdb_close(db);

        printf("%s\n", s1);
    }

    return 0;
}