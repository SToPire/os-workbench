#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

struct kvdb {
    int fd;
};
typedef struct kvdb kvdb_t;

struct kvdb* kvdb_open(const char* filename)
{
    kvdb_t* ptr = malloc(sizeof(kvdb_t));
    ptr->fd = open(filename, O_CREAT | O_RDWR);
    return ptr;
}

int kvdb_close(struct kvdb* db)
{
    return -1;
}

int kvdb_put(struct kvdb* db, const char* key, const char* value)
{
    return -1;
}

char* kvdb_get(struct kvdb* db, const char* key)
{
    return NULL;
}
