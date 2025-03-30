#include "filedb.h"

#include "config.h"
#include "util.h"

#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <sha1.h>

filedb_t*
filedb_new()
{
    filedb_t *db = malloc(sizeof(filedb_t));
    db->size = 0;
    db->capacity = 64;
    db->vec = malloc(sizeof(file_info_t) * db->capacity);
    return db;
}

file_info_t*
filedb_insert(filedb_t *db, const char *name, const char *hash, size_t size)
{
    if (db->size + 1 > db->capacity) {
        db->capacity *= 2;
        db->vec = realloc(db->vec, db->capacity);
    }

    db->vec[db->size].name = name;
    db->vec[db->size].hash = hash;
    db->vec[db->size].size = size;
    db->size++;

    return &db->vec[db->size - 1];
}

server_list_t*
sl_new()
{
    server_list_t *sl = malloc(sizeof(server_list_t));
    sl->size = 0;
    sl->capacity = INITIAL_VECTOR_CAPACITY;
    sl->vec = malloc(sizeof(char*) * sl->capacity);
    return sl;
}

void
sl_insert(server_list_t *sl, const char *hostname)
{
    if (sl->size + 1 > sl->capacity) {
        sl->capacity *= 2;
        sl->vec = realloc(sl->vec, sl->capacity);
    }

    sl->vec[sl->size] = hostname;
    sl->size++;
}

const char*
compute_file_hash(const char *filename)
{
    static char hashstr[256];

    NF_TRY(
        SHA1File(filename, hashstr) == NULL,
        "SHA1File", strerror(errno), return NULL
    );

    return hashstr;
}

size_t
get_file_size(const char *filename)
{
    struct stat statbuf;
    NF_TRY_C(
        stat(filename, &statbuf) < 0,
        "stat", strerror(errno), filename, return 0
    );

    NF_TRY_C(
        !S_ISREG(statbuf.st_mode),
        "stat", "Not a regular file", filename, return 0
    );

    return statbuf.st_size;

}

int
filedb_scan(filedb_t *db, const char *dirpath)
{
    DIR *dir = NULL;
    static char buff[4096];

    NF_TRY_C(
        (dir = opendir(dirpath)) == NULL,
        "opendir", strerror(errno), dirpath, return -1
    );
    
    struct dirent *de = NULL;
    errno = 0;
    while ((de = readdir(dir))) {
        NF_TRY(errno != 0, "readdir", strerror(errno), return -1);
        errno = 0;

        switch (de->d_type) {
            case DT_DIR: {
                if (de->d_name[0] == '.' && (de->d_name[1] == '\0' ||
                    (de->d_name[1] == '.' && de->d_name[2] == '\0')))   
                        continue;
                snprintf(buff, 4096, "%s%s/", dirpath, de->d_name);
                filedb_scan(db, buff);
            } break;
            case DT_REG: {
                snprintf(buff, 4096, "%s%s", dirpath, de->d_name);
                filedb_insert(db,
                    strdup(buff),
                    strdup(compute_file_hash(buff)),
                    get_file_size(buff)
                );
            } break;
        }
    }

    closedir(dir);

    return 0;
}

int
filedb_find(const filedb_t *db, const char *hash)
{
    for (int i = 0; i < db->size; i++) {
        if (strcmp(db->vec[i].hash, hash) == 0)
            return i;
    }
    return -1;
}

void
filedb_print(const filedb_t *db, FILE *f)
{
    fprintf(f, "files:\t%-*s\tsize\tfilename\n", 40, "hash");
    for (int i = 0; i < db->size; i++) {
        printf("\t%s\t%ld\t%s\n", db->vec[i].hash, db->vec[i].size,
            db->vec[i].name);
    }
}

void
filedb_destroy(filedb_t *db)
{
    for (int i = 0; i < db->size; i++) {
        free((char*)db->vec[i].hash);
        free((char*)db->vec[i].name);
        if (db->vec[i].serverlist) {
            for (int j = 0; j < db->vec[i].serverlist->size; j++)
                free((char*)db->vec[i].serverlist->vec[j]);
            free(db->vec[i].serverlist);
        }
    }
    free(db->vec);
}

