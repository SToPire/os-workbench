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
        kvdb_put(db, "aaa", "bbb");
        kvdb_close(db);
    } else {
        while (waitpid(pid, NULL, WNOHANG) != pid)
            ;
        struct kvdb* db = kvdb_open("miao.db");
        kvdb_put(db, "eeee", "ffff");
        char* s2 = kvdb_get(db, "eeee");
        char* s1 = kvdb_get(db, "aaa");
        kvdb_put(db, "gg", "h");
        // char* s3 = kvdb_get(db, "gg");
        kvdb_close(db);

        printf("%s %s\n", s1, s2);
        //printf("%s %s %s\n", s1, s2,s3);
    }

    return 0;
}