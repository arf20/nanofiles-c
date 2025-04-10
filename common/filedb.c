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
    db->vec[db->size].peerlist = NULL;
    db->size++;

    return &db->vec[db->size - 1];
}

file_info_t*
filedb_find_name(const filedb_t *db, const char *name)
{
    for (int i = 0; i < db->size; i++)
        if (strcmp(db->vec[i].name, name) == 0)
            return &db->vec[i];
    return NULL;
}

file_info_t*
filedb_find_hash(const filedb_t *db, const char *hash)
{
    for (int i = 0; i < db->size; i++)
        if (strcmp(db->vec[i].hash, hash) == 0)
            return &db->vec[i];
    return NULL;
}

peer_list_t*
sl_new()
{
    peer_list_t *sl = malloc(sizeof(peer_list_t));
    sl->size = 0;
    sl->capacity = INITIAL_VECTOR_CAPACITY;
    sl->vec = malloc(sizeof(char*) * sl->capacity);
    return sl;
}

void
sl_insert(peer_list_t *sl, const char *hostname)
{
    if (sl->size + 1 > sl->capacity) {
        sl->capacity *= 2;
        sl->vec = realloc(sl->vec, sl->capacity);
    }

    sl->vec[sl->size] = hostname;
    sl->size++;
}

int
sl_exists(peer_list_t *sl, const char *hostname)
{
    for (int i = 0; i < sl->size; i++)
        if (strcmp(sl->vec[i], hostname) == 0)
            return 1;
    return 0;
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
filedb_scan(filedb_t *db, const char *basepath, const char *dirpath)
{
    DIR *dir = NULL;
    static char buff[4096] = { 0 };

    snprintf(buff, 4096, "%s%s", basepath, dirpath);

    NF_TRY_C(
        (dir = opendir(buff)) == NULL,
        "opendir", strerror(errno), buff, return -1
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
                char *nextdir = strdup(buff);
                filedb_scan(db, basepath, nextdir);
                free(nextdir);
            } break;
            case DT_REG: {
                snprintf(buff, 4096, "%s%s%s", basepath, dirpath, de->d_name);
                size_t fsize = get_file_size(buff);
                const char *fhash = strdup(compute_file_hash(buff));
                snprintf(buff, 4096, "%s%s", dirpath, de->d_name);

                filedb_insert(db, strdup(buff), fhash, fsize);
            } break;
        }
    }

    closedir(dir);

    return 0;
}

void
filedb_print(const filedb_t *db, FILE *f)
{
    fprintf(f, "files:\t%-*s\tsize\tfilename\tservers\n", 40, "hash");
    for (int i = 0; i < db->size; i++) {
        printf("\t%s\t%ld\t%s", db->vec[i].hash, db->vec[i].size,
            db->vec[i].name);
        if (db->vec[i].peerlist) {
            printf("\t");
            for (int j = 0; j < db->vec[i].peerlist->size; j++)
                printf(" %s", db->vec[i].peerlist->vec[j]);
        }
        printf("\n");
    }
}

void
filedb_destroy(filedb_t *db)
{
    for (int i = 0; i < db->size; i++) {
        free((char*)db->vec[i].hash);
        free((char*)db->vec[i].name);
        if (db->vec[i].peerlist) {
            for (int j = 0; j < db->vec[i].peerlist->size; j++)
                free((char*)db->vec[i].peerlist->vec[j]);
            free(db->vec[i].peerlist);
        }
    }
    free(db->vec);
    free(db);
}

