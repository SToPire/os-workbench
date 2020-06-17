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

#define key_size 128
#define value_size 16 * 1024 * 1024
#define entry_size key_size + value_size + 32

struct kvdb {
    int fd;
    pthread_mutex_t* mutex;
};
typedef struct kvdb kvdb_t;

void To_End_of_DB(int fd)
{
    char tmp;
    lseek(fd, -1, SEEK_END);
    read(fd, &tmp, 1);
    while (tmp != '\n') {
        lseek(fd, -2, SEEK_CUR);
        read(fd, &tmp, 1);
    }
}

struct kvdb* kvdb_open(const char* filename)
{
    int exist = 0;
    if (access(filename, F_OK) == 0) exist = 1;
    kvdb_t* db = malloc(sizeof(kvdb_t));
    if ((db->fd = open(filename, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO)) < 0) assert(0);
    db->mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(db->mutex, NULL);

    if (!exist) {
        char* tmp = malloc(entry_size);
        tmp[0] = '!';
        tmp[entry_size - 1] = '\n';
        lseek(db->fd, 0, SEEK_SET);
        write(db->fd, tmp, entry_size);
        fsync(db->fd);
        lseek(db->fd, 0, SEEK_SET);
        free(tmp);
    }
    return db;
}

int kvdb_close(struct kvdb* db)
{
    close(db->fd);
    free(db->mutex);
    free(db);
    return 0;
}
void CheckAndRepair(kvdb_t* db)
{
    lseek(db->fd, 0, SEEK_SET);
    char t;
    read(db->fd, &t, 1);
    if (t == 'M') {
        char* tmp = malloc(entry_size);
        char* tmp_key = malloc(key_size);
        char* tmp_value = malloc(value_size);
        read(db->fd, tmp, entry_size);

        int tmp_ptr = 0, cur = 0;
        while (tmp[cur] != ' ') tmp_key[tmp_ptr++] = tmp[cur++];
        tmp_key[tmp_ptr] = 0;
        tmp_ptr = 0;
        cur++;
        while (tmp[cur] != '\n') tmp_value[tmp_ptr++] = tmp[cur++];
        tmp_value[tmp_ptr] = 0;

        To_End_of_DB(db->fd);
        write(db->fd, tmp_key, strlen(tmp_key));
        write(db->fd, " ", 1);
        write(db->fd, tmp_value, strlen(tmp_value));
        write(db->fd, "\n", 1);
        fsync(db->fd);
        lseek(db->fd, 0, SEEK_SET);
        write(db->fd, "!", 1);

        free(tmp);
        free(tmp_key);
        free(tmp_value);
    }
}
int kvdb_put(struct kvdb* db, const char* key, const char* value)
{
    pthread_mutex_lock(db->mutex);
    flock(db->fd, LOCK_EX);

    CheckAndRepair(db);

    lseek(db->fd, 0, SEEK_SET);
    write(db->fd, "M", 1);
    write(db->fd, key, strlen(key));
    write(db->fd, " ", 1);
    write(db->fd, value, strlen(value));
    write(db->fd, "\n", 1);

    fsync(db->fd);

    CheckAndRepair(db);

    flock(db->fd, LOCK_UN);
    pthread_mutex_unlock(db->mutex);
    return 0;
}

char* kvdb_get(struct kvdb* db, const char* key)
{
    pthread_mutex_lock(db->mutex);
    flock(db->fd, LOCK_EX);

    CheckAndRepair(db);

    char* buf = malloc(entry_size);
    char* tmp_key = malloc(key_size);
    char* tmp_value = malloc(value_size);
    char* ret = NULL;

    off_t offset = lseek(db->fd, entry_size, SEEK_SET);
    int len = read(db->fd, buf, entry_size), cur = 0;
    while (len > 0 && buf[len - 1] != '\n') --len;

    while (1) {
        int cur = 0;
        memset(buf, 0, entry_size);
        lseek(db->fd, offset, SEEK_SET);
        int len = read(db->fd, buf, entry_size);
        while (len > 0 && buf[len - 1] != '\n') --len;

        if (len == 0) break;

        while (cur < len) {
            int tmp_ptr = 0;
            while (buf[cur] != ' ') tmp_key[tmp_ptr++] = buf[cur++];
            tmp_key[tmp_ptr] = '\0';
            cur++, tmp_ptr = 0;
            while (buf[cur] != '\n') tmp_value[tmp_ptr++] = buf[cur++];
            tmp_value[tmp_ptr] = '\0';
            cur++;
            if (strcmp(tmp_key, key) == 0) {
                ret = malloc(strlen(tmp_value) + 1);
                strcpy(ret, tmp_value);
            }
        }
        offset += len;
    }
    free(buf);
    free(tmp_key);
    free(tmp_value);

    flock(db->fd, LOCK_UN);
    pthread_mutex_unlock(db->mutex);
    return ret;
}
