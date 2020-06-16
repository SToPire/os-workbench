#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <pthread.h>
#include <errno.h>

struct kvdb {
    int fd;
    pthread_mutex_t* mutex;
};
typedef struct kvdb kvdb_t;

#define reserved_sz 1024*1024

off_t ltell(int fd)
{
    return lseek(fd, 0, SEEK_CUR);
}

void kvdb_lock(kvdb_t* db)
{
    pthread_mutex_consistent(db->mutex);
    flock(db->fd, LOCK_EX);
}

void kvdb_unlock(kvdb_t* db)
{
    flock(db->fd, LOCK_UN);
    pthread_mutex_unlock(db->mutex);
}

struct kvdb* kvdb_open(const char* filename)
{
    kvdb_t* db = malloc(sizeof(kvdb_t));
    db->fd = open(filename, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXU);
    db->mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(db->mutex, NULL);

    if(lseek(db->fd,0,SEEK_END) < reserved_sz){
        char* tmp = malloc(reserved_sz);
        tmp[0] = 'N';
        tmp[1] = '\n';
        tmp[reserved_sz - 1] = '\n';
        lseek(db->fd, 0, SEEK_SET);
        write(db->fd, tmp, reserved_sz);
        lseek(db->fd, 0, SEEK_SET);
    }
    return db;
}

int kvdb_close(struct kvdb* db)
{
    close(db->fd);
    free(db->mutex);
    return 0;
}

void kvdb_journal(kvdb_t* db){
    lseek(db->fd, 0, SEEK_SET);
    char* buf = malloc(reserved_sz);
    read(db->fd, buf, 2);
    if (buf[0] == 'Y') {
        read(db->fd, buf, reserved_sz);
        char* key = malloc(1024);
        char* val = malloc(4096);
        sscanf(buf, " %s %s", key, val);

        lseek(db->fd, 0, SEEK_END);
        lseek(db->fd, -1, SEEK_CUR);
        char buf;
        read(db->fd, &buf, 1);
        while (buf != '\n') {
            lseek(db->fd, -2, SEEK_CUR);
            read(db->fd, &buf, 1);
        }
        write(db->fd, key, strlen(key));
        write(db->fd, " ", 1);
        write(db->fd, val, strlen(val));
        write(db->fd, "\n", 1);
        //sync();
        free(key);
        free(val);
        lseek(db->fd, 0, SEEK_SET);
        write(db->fd, "N\n", 2);
    }
    free(buf);
}

int kvdb_put(struct kvdb* db, const char* key, const char* value)
{
    kvdb_lock(db);

    // char buf[4096 + 128];
    // int charCnt = sprintf(buf, "%s %s\n", key, value);
    // write(db->fd, buf, charCnt);
    // fsync(db->fd);

    kvdb_journal(db);
    lseek(db->fd, 2, SEEK_SET);
    write(db->fd, key, strlen(key));
    write(db->fd, " ", 1);
    write(db->fd, value, strlen(value));
    write(db->fd, "\n", 1);
    lseek(db->fd, 0, SEEK_SET);
    write(db->fd, "Y\n", 2);
    //sync();
    kvdb_journal(db);

    kvdb_unlock(db);
    return 0;
}

char* kvdb_get(struct kvdb* db, const char* key)
{
    kvdb_lock(db);

    // char* ret = NULL;
    // off_t old_off = lseek(db->fd, 0, SEEK_CUR);
    // off_t END = lseek(db->fd, 0, SEEK_END);
    // lseek(db->fd, 0, SEEK_SET);

    // char buf[4096 + 128];

    // while (ltell(db->fd) != END) {
    //     memset(buf, 0, 4096 + 128);
    //     for (int i = 0;; ++i) {
    //         read(db->fd, buf + i, 1);
    //         if (buf[i] == '\n') break;
    //     }
    //     int space = 0, enter = 0;
    //     while (buf[++space] != ' ')
    //         ;
    //     while (buf[++enter] != '\n')
    //         ;

    //     if (strlen(key) == space && strncmp(buf, key, space) == 0) {
    //         ret = malloc(enter - space);
    //         strncpy(ret, buf + space + 1, enter - space - 1);
    //         ret[enter - space - 1] = '\0';
    //     }
    // }

    // lseek(db->fd, old_off, SEEK_SET);

    char* buf = malloc(reserved_sz);
    char* rkey = malloc(1024);
    char* rval = malloc(1024);
    char* ret = calloc(1, 1024);

    kvdb_journal(db);
    find_start(db->fd);
    off_t offset = lseek(db->fd, 0, SEEK_CUR);
    while (read(db->fd, buf, reserved_sz)) {
        sscanf(buf, " %s %s", rkey, rval);
        if (!strcmp(key, rkey)) {
            strcpy(ret, rval);
        }
        offset += strlen(rkey) + strlen(rval) + 2;
        lseek(db->fd, offset, SEEK_SET);
    }

    free(buf);
    free(rkey);
    free(rval);
    kvdb_unlock(db);
    return ret;
}
