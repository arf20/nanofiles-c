#ifndef _FILEDB_H
#define _FILEDB_H

#include <stddef.h>
#include <stdio.h>

typedef struct {
    const char **vec;
    size_t size, capacity;
} server_list_t;

typedef struct {
    const char *name; /* includes path */
    const char *hash;
    size_t size;
    server_list_t *serverlist;
} file_info_t;

typedef struct {
    size_t size, capacity;
    file_info_t *vec;
} filedb_t;

filedb_t* filedb_new();
file_info_t* filedb_insert(filedb_t *db, const char *name,
    const char *hash, size_t size);
file_info_t* filedb_find_name(filedb_t *db, const char *name);
file_info_t* filedb_find_hash(filedb_t *db, const char *hash);
server_list_t* sl_new();
void sl_insert(server_list_t *sl, const char *hostname);
int sl_exists(server_list_t *sl, const char *hostname);
int filedb_scan(filedb_t *db, const char *basepath, const char *dirpath);
void filedb_print(const filedb_t *db, FILE *f);
void filedb_destroy(filedb_t *db);

#endif /* _FILEDB_H */

